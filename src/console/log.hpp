#ifndef ___console_log___
#define ___console_log___

#include <ostream>
#include <stdio.h>
#include <string>

namespace sst { class dict; }

namespace console {

class iLogSink {
public:
   virtual void writeWords(const std::string& words) = 0;
   virtual void flush() = 0;
};

class cStdOutLogSink : public iLogSink {
public:
   virtual void writeWords(const std::string& words) { ::printf(words.c_str()); }
   virtual void flush() { ::_flushall(); }
};

class cppStreamLogSink : public iLogSink {
public:
   explicit cppStreamLogSink(std::ostream& o) : m_out(o) {}

   virtual void writeWords(const std::string& words) { m_out << words; }
   virtual void flush() { m_out << std::flush; }

private:
   std::ostream& m_out;
};

class iLog {
public:
   virtual ~iLog() {}

   virtual void configure(sst::dict& d) = 0;
   virtual void configureForBackground() = 0;
   virtual void sink(iLogSink *pSink) = 0;

   virtual void adjustIndent(int i) = 0;

   virtual void writeLnInfo(const std::string& fmt, ...) = 0;
   virtual void writeLnVerbose(const std::string& fmt, ...) = 0;
   virtual void writeLnDebug(const std::string& fmt, ...) = 0;
   virtual void writeLnTemp(const std::string& fmt, ...) = 0;
};

class nullLog : public iLog {
public:
   virtual void configure(sst::dict& d) {}
   virtual void configureForBackground() {}
   virtual void sink(iLogSink *pSink) {}

   virtual void adjustIndent(int i) {}

   virtual void writeLnInfo(const std::string& fmt, ...) {}
   virtual void writeLnVerbose(const std::string& fmt, ...) {}
   virtual void writeLnDebug(const std::string& fmt, ...) {}
   virtual void writeLnTemp(const std::string& fmt, ...) {}
};

class autoIndent {
public:
   autoIndent(iLog& l, int offset = 3)
   : m_l(l) , m_offset(offset * -1)
   { l.adjustIndent(offset); }

   ~autoIndent()
   { m_l.adjustIndent(m_offset); }

private:
   iLog& m_l;
   int m_offset;

   autoIndent(const autoIndent&);
   autoIndent& operator=(const autoIndent&);
};

} // namespace console

#endif // ___console_log___
