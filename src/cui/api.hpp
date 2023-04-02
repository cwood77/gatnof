#ifndef ___cui_api___
#define ___cui_api___

#include <functional>
#include <map>
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
   control() : m_p(0,0), m_l(0) {}

   virtual void release() { delete this; }
   void initialize(pnt p, size_t l);

   std::string get() { return m_str; }
   void update(const std::string& v);

   void access(std::function<void(std::string&)> f);

protected:
   virtual void formatText(std::ostream& o) {}

private:
   pnt m_p;
   size_t m_l;
   std::string m_str;
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
