#ifndef ___console_arg___
#define ___console_arg___

#include <list>
#include <set>
#include <stdexcept>
#include <string>

namespace console {

class iLog;

// result of parsing - user defined
class iCommand {
public:
   virtual ~iCommand() {}
   virtual void run(iLog& l) {}
};

// participates in parsing
class iArgPattern {
public:
   virtual ~iArgPattern() {}
   virtual bool tryMatchWord(const std::string& word) = 0;
   virtual void adjustPatterns(std::list<iArgPattern*>& list) = 0;
   virtual iCommand *complete() = 0;
};

// the top-level parsing object
class iCommandLineParser {
public:
   virtual ~iCommandLineParser() {}
   virtual iCommandLineParser& addVerb(iArgPattern& v) = 0;
   virtual iCommand *parse(int argc, const char *argv[]) = 0;
};

// participates in post-parse validation
class iCommandVerifier : public iArgPattern {
public:
   virtual void verify() = 0;
};

// programs the command based on the parse
class iCommandConfig : public iCommandVerifier {
public:
   virtual void collaborate(iCommand& c, iCommandVerifier& verb) = 0;
};

// a command-line input
class stringParameter : public iCommandConfig {
public:
   static stringParameter& optional(size_t offset);
   static stringParameter& required(size_t offset);

   virtual bool tryMatchWord(const std::string& word);
   virtual void adjustPatterns(std::list<iArgPattern*>& list);
   virtual iCommand *complete();
   virtual void verify();
   virtual void collaborate(iCommand& c, iCommandVerifier& verb);

private:
   stringParameter(size_t offset, bool required);

   size_t m_offset;
   bool m_required;
   iCommand *m_pCmd;
   iCommandVerifier *m_pVerb;
   bool m_read;
};

// a command-line switch
class iOption : public iCommandConfig {
public:
   virtual iOption& addTag(const std::string& tag) = 0;
};

// a boolean command-line switch
class boolOption : public iOption {
public:
   boolOption(const std::string& tag, size_t offset);

   virtual bool tryMatchWord(const std::string& word);
   virtual void adjustPatterns(std::list<iArgPattern*>& list);
   virtual iCommand *complete();
   virtual void verify() {}
   virtual void collaborate(iCommand& c, iCommandVerifier& verb);
   virtual iOption& addTag(const std::string& tag);

private:
   std::set<std::string> m_tags;
   size_t m_offset;
   iCommand *m_pCmd;
   iCommandVerifier *m_pVerb;
};

// verbs own options, parameters, and commands, but nobody owns verbs
class verbBase : public iCommandVerifier {
public:
   virtual ~verbBase();

   verbBase& addParameter(iCommandConfig& c);
   iOption& addOption(iOption& o);

   virtual bool tryMatchWord(const std::string& word);
   virtual void adjustPatterns(std::list<iArgPattern*>& list);
   virtual iCommand *complete();
   virtual void verify();

protected:
   verbBase(iCommand& c, const std::string& tag);

private:
   iCommand *m_pCmd;
   std::string m_tag;
   std::list<iCommandConfig*> m_params;
   std::list<iCommandConfig*> m_options;
};

template<class T>
class verb : public verbBase {
public:
   verb(const std::string& tag) : verbBase(*new T(), tag) {}
};

class iGlobalVerb {
public:
   virtual void program(iCommandLineParser& p) = 0;
   virtual void deflate() = 0;
};

class verbsGlobal {
public:
   static verbsGlobal& get();

   void add(iGlobalVerb& v);

   void program(iCommandLineParser& p);
   void deflate();

private:
   std::list<iGlobalVerb*> m_verbs;
};

class globalVerb : public iGlobalVerb {
public:
   globalVerb();

   virtual void program(iCommandLineParser& p);
   virtual void deflate();

protected:
   virtual verbBase *inflate() = 0;

private:
   verbBase *m_pVerb;
};

class autoVerbs {
public:
   ~autoVerbs();

   void program(iCommandLineParser& p);
};

#include "arg.ipp"

} // namespace console

#endif // ___console_arg___
