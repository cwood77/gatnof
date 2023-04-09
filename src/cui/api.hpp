#ifndef ___cui_api___
#define ___cui_api___

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace cui { 

// --------------- infrastructure

class iObject {
public:
   virtual ~iObject() {}
   virtual void release() = 0;
};

// used to locate images/screens
class iFactory {
public:
   virtual ~iFactory() {}
   template<class T> T& create(const std::string& id)
   { return dynamic_cast<T&>(_create(typeid(T).name(),id)); }
   virtual iObject& _create(const std::string& typeId, const std::string& id) = 0;
};

class iPlugInFactory {
public:
   virtual ~iPlugInFactory() {}
   virtual iObject *tryCreate(const std::string& typeId, const std::string& id) = 0;
};

template<class T, class I>
class plugInFactoryT : public iPlugInFactory {
public:
   explicit plugInFactoryT(const std::string& id) : m_id(id) {}

   virtual iObject *tryCreate(const std::string& typeId, const std::string& id)
   {
      if(typeId != typeid(I).name())
         return NULL;
      if(id != m_id)
         return NULL;
      return new T();
   }

private:
   std::string m_id;
};

// --------------- simple atomics

class pnt {
public:
   pnt(size_t x, size_t y) : x(x), y(y) {}

   size_t x;
   size_t y;
};

// --------------- toolbox of controls, etc.

class control : public iObject {
public:
   control() : m_p(0,0), m_l(0), m_h(0), m_i(1), m_userInit(false) {}

   virtual void release() { delete this; }
   void initialize(pnt p, size_t l, size_t h);
   void userInitialize(std::function<void(void)> f);

   const pnt& getLoc() const { return m_p; }
   size_t getLength() const { return m_l; }
   size_t getHeight() const { return m_h; }

   void erase();
   void setFormatMode(size_t i) { m_i = i; }
   size_t getFormatMode() const { return m_i; }
   void formatText(std::ostream& o);

protected:
   virtual void onInitialize() { erase(); }

   virtual void formatText1(std::ostream& o) {}
   virtual void formatText2(std::ostream& o) {}
   virtual void formatText3(std::ostream& o) {}
   virtual void formatText4(std::ostream& o) {}

private:
   pnt m_p;
   size_t m_l;
   size_t m_h;
   size_t m_i;
   bool m_userInit;
};

class stringControl : public control {
public:
   std::string get() { return m_cache; }
   void redraw(const std::string& v);

   void update(const std::string& v);

private:
   std::string m_cache;
};

class iIntFormatter {
public:
   virtual std::string formatValue(int v, size_t l) const = 0;
};

class intControl : public control {
public:
   intControl();
   int get() { return m_cache; }
   void redraw(int v);

   void update(int v);

   void setFormatter(iIntFormatter& f) { m_pFmt.reset(&f); }

private:
   int m_cache;
   std::unique_ptr<iIntFormatter> m_pFmt;
};

class maxValueIntFormatter : public iIntFormatter {
public:
   virtual std::string formatValue(int v, size_t l) const;
};

class bracketedIntFormatter : public iIntFormatter {
public:
   explicit bracketedIntFormatter(iIntFormatter& next) : m_pNext(&next) {}

   virtual std::string formatValue(int v, size_t l) const;

private:
   std::unique_ptr<iIntFormatter> m_pNext;
};

class hugeValueIntFormatter : public iIntFormatter {
public:
   virtual std::string formatValue(int v, size_t l) const;

private:
   std::string abbreviate(
      const std::string& s, size_t nBeforeDot, size_t l, char unit) const;
};

class buttonControl : public control {
public:
   void setFaceText(const std::string& t);
   void dim(const std::string& reason) { setFormatMode(2); m_dimReason = reason; }

   bool isEnabled() const { return getFormatMode() == 1; }
   char getCmdKey() const { return m_cmd; }
   const std::string getDimReason() const { return m_dimReason; }

   void redraw();

protected:
   virtual void onInitialize() { redraw(); }

private:
   std::string m_beforeCmdText;
   std::string m_cmdText;
   std::string m_afterCmdText;
   char m_cmd;
   std::string m_dimReason;
};

template<class T>
class listControl : public iObject {
public:
   virtual void release() { delete this; }

   void initialize(pnt, size_t, size_t nRows)
   {
      m_rows.resize(nRows);
      for(size_t i=0;i<nRows;i++)
         m_rows[i].initialize(i);
   }

   size_t size() const { return m_rows.size(); }
   T& operator[](size_t i) { return m_rows[i]; }

private:
   std::vector<T> m_rows;
};

// --------------- bases of codegened specifics

class iImage : public iObject {
public:
   virtual void release() { delete this; }
   virtual void render() = 0;
   virtual pnt demandPnt(const std::string& id) = 0;
};

class iScreen : public virtual iObject {
public:
   virtual void render() = 0;
   virtual iObject& _demand(const std::string& id) = 0;
   template<class T> T& demand(const std::string& id)
   { return dynamic_cast<T&>(_demand(id)); }
};

class basicScreen : public iScreen {
public:
   virtual void release() { delete this; }
   virtual iObject& _demand(const std::string& id);

protected:
   void publishObject(const std::string& id, iObject& o);

private:
   std::map<std::string,iObject*> m_map;
};

// --------------- input

class iUserInput {
public:
   virtual ~iUserInput() {}
   virtual char getKey() = 0;
};

class userInputSyphon : public iUserInput { // TODO unused
public:
   explicit userInputSyphon(iUserInput& u) : m_inner(u) {}

   void onKey(char c, std::function<void(void)> f);
   virtual char getKey();

private:
   iUserInput& m_inner;
   std::map<char,std::function<void(void)> > m_funcs;
};

class buttonHandler {
public:
   explicit buttonHandler(stringControl& error) : m_error(error) {}

   // return true to stop
   void add(buttonControl& b, std::function<void(bool&)> f);
   void addCustom(char k, std::function<void(bool&)> f);
   void unimpled(buttonControl& b);

   buttonControl *run(iUserInput& in);

private:
   stringControl& m_error;
   std::map<char,buttonControl*> m_btns;
   std::map<char,std::function<void(bool&)> > m_callbacks;
};

// --------------- hand-written top-levels

class iLogic : public virtual iObject {
public:
   virtual void release() { delete this; }
   virtual void run(bool interactive = true) = 0;
};

} // namespace cui

#endif // ___cui_api___
