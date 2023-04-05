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

protected:
   void formatText(std::ostream& o);
   virtual void formatText1(std::ostream& o) {}
   virtual void formatText2(std::ostream& o) {}

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

// --------------- bases of codegened specifics

class iImage : public iObject {
public:
   virtual void release() { delete this; }
   virtual void render() = 0;
   virtual pnt demandPnt(const std::string& id) = 0;
};

class iScreen : public iObject {
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

class iLogic : public iObject {
public:
   virtual void release() { delete this; }
   virtual void run() = 0;
};

} // namespace cui

#endif // ___cui_api___
