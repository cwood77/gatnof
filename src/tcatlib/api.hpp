#ifndef ___tcatlib_api___
#define ___tcatlib_api___

#include "../tcatbin/api.hpp"
#include <stdint.h>
#include <string>
#include <typeinfo>
#include <vector>

namespace tcat {

// translate return values into exceptions
class catalogWrapper : public tcatbin::iCatalog {
public:
   catalogWrapper();
   void set(tcatbin::iCatalog *pInner);

   virtual void *createSingleType(const char *pTypeName);
   virtual void *createMultipleTypes(const char *pTypeName, size_t& n);
   virtual void releaseType(void *pPtr);
   virtual void releaseMultipleTypes(void *pPtr, size_t n);

private:
   tcatbin::iCatalog *m_pInner;
};

// loads/unloads the tcatbin library
class libStub {
public:
   static libStub& get();

   void addref();
   void release();

   tcatbin::iCatalog& getCat() { return m_cat; }

private:
   libStub();

   uint32_t m_refCnt;
   void *m_unloadFunc;
   void *m_dllPtr;
   catalogWrapper m_cat;
};

// refcounts on libStub
class libRef {
public:
   libRef();
   ~libRef();
   libRef(const libRef& source);
   libRef& operator=(const libRef& source);

   tcatbin::iCatalog& getCat() { return m_pPtr->getCat(); }

private:
   libStub *m_pPtr;
};

// the singlecast type accessor class
template<class T>
class typePtr {
public:
   explicit typePtr(T *pPtr = NULL) : m_pPtr(pPtr)
   {
      if(!m_pPtr)
         m_pPtr = (T*)m_libRef.getCat().createSingleType(typeid(T).name());
   }

   explicit typePtr(const std::string& type)
   {
      m_pPtr = (T*)m_libRef.getCat().createSingleType(type.c_str());
   }

   ~typePtr()
   {
      if(m_pPtr)
         m_libRef.getCat().releaseType(m_pPtr);
   }

   void reset(T *pPtr)
   {
      if(m_pPtr)
         m_libRef.getCat().releaseType(m_pPtr);
      m_pPtr = pPtr;
   }

   T *operator->() { return m_pPtr; }

   T& operator*() { return *m_pPtr; }

   T *get() { return m_pPtr; }

   T *leak() { T *pTmp = m_pPtr; m_pPtr = NULL; return pTmp; }

private:
   libRef m_libRef;
   T *m_pPtr;

   template<class O> typePtr(const typePtr<O>& source);
   template<class O> typePtr<O>& operator=(const typePtr<O>& source);
};

// the multicast type accessor class
template<class T>
class typeSet {
public:
   typeSet()
   {
      m_pPtr = (T**)m_libRef.getCat().createMultipleTypes(typeid(T).name(),m_size);
   }

   ~typeSet()
   {
      m_libRef.getCat().releaseMultipleTypes((void*)m_pPtr,m_size);
   }

   size_t size() const { return m_size; }

   T *operator[](size_t i) { return m_pPtr[i]; }

private:
   libRef m_libRef;
   T **m_pPtr;
   size_t m_size;
};

// simple singleton module server
class staticModuleServer : public tcatbin::iModuleServer {
public:
   static staticModuleServer& get();
   void add(tcatbin::iTypeServer& t);

   virtual size_t getNumTypes() const;
   virtual tcatbin::iTypeServer& getIthType(size_t i);

private:
   std::vector<tcatbin::iTypeServer*> m_types;
};

// type server for simple types (flyweight)
template<class I, class T>
class staticTypeServer : public tcatbin::iTypeServer {
public:
   staticTypeServer() { staticModuleServer::get().add(*this); }
   virtual const char *getTypeName() const { return typeid(I).name(); }
   virtual size_t getFlags() const { return 0; }
   virtual void *createType() { return (I*) new T(); }
   virtual void releaseType(void *p) { delete (T*)p; }
};

// type serve for singleton types
template<class I, class T>
class staticSingletonTypeServer : public tcatbin::iTypeServer {
public:
   staticSingletonTypeServer() { staticModuleServer::get().add(*this); }
   virtual const char *getTypeName() const { return typeid(I).name(); }
   virtual size_t getFlags() const { return kSingleton; }
   virtual void *createType() { return (I*) new T(); }
   virtual void releaseType(void *p) { delete (T*)p; }
};

#define tcatExposeSingletonTypeAs(__type__,__intf__) \
   tcat::staticSingletonTypeServer<__intf__,__type__> __typeServerIntf##__type__;

#define tcatExposeTypeAs(__type__,__intf__) \
   tcat::staticTypeServer<__intf__,__type__> __typeServerIntf##__type__;

#define tcatImplServer() \
   __declspec(dllexport) tcatbin::iModuleServer *getModuleServer() \
   { \
      return &tcat::staticModuleServer::get(); \
   }

} // namespace tcat

#endif // ___tcatlib_api___
