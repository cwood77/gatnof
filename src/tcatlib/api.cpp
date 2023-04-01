#include "api.hpp"
#include <stdexcept>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tcat {

catalogWrapper::catalogWrapper()
: m_pInner(NULL)
{
}

void catalogWrapper::set(tcatbin::iCatalog *pInner)
{
   m_pInner = pInner;
}

void *catalogWrapper::createSingleType(const char *pTypeName)
{
   if(!m_pInner)
      throw std::runtime_error("attempt to use NULL iCatalog");
   void *pRval = m_pInner->createSingleType(pTypeName);
   if(!pRval)
      throw std::runtime_error("cannot locate type in createSingleType");
   return pRval;
}

void *catalogWrapper::createMultipleTypes(const char *pTypeName, size_t& n)
{
   if(!m_pInner)
      throw std::runtime_error("attempt to use NULL iCatalog");
   return m_pInner->createMultipleTypes(pTypeName,n);
}

void catalogWrapper::releaseType(void *pPtr)
{
   if(!m_pInner)
      throw std::runtime_error("attempt to use NULL iCatalog");
   m_pInner->releaseType(pPtr);
}

void catalogWrapper::releaseMultipleTypes(void *pPtr, size_t n)
{
   if(!m_pInner)
      throw std::runtime_error("attempt to use NULL iCatalog");
   m_pInner->releaseMultipleTypes(pPtr,n);
}

libStub& libStub::get()
{
   static libStub theStub;
   return theStub;
}

void libStub::addref()
{
   if(::InterlockedIncrement((LONG*)&m_refCnt) == 1)
   {
      char path[MAX_PATH];
      ::GetModuleFileNameA(NULL,path,MAX_PATH);
      std::string tcat = path;
      tcat += "\\..\\tcatbin.dll";
      m_dllPtr = ::LoadLibraryA(tcat.c_str());
      if(!m_dllPtr)
         throw std::runtime_error("tcatbin not found");

      typedef tcatbin::iCatalog& (*createFunc_t)();
      createFunc_t func = (createFunc_t)::GetProcAddress((HMODULE)m_dllPtr,"_ZN7tcatbin8iCatalog6createEv");
      if(!func)
         throw std::runtime_error("tcatbin incompatible - no create func");

      m_unloadFunc = (void*)::GetProcAddress((HMODULE)m_dllPtr,"_ZN7tcatbin8iCatalog7destroyEv");
      if(!m_unloadFunc)
         throw std::runtime_error("tcatbin incompatible - no destroy func");

      m_cat.set(&func());
   }
}

void libStub::release()
{
   if(::InterlockedDecrement((LONG*)&m_refCnt) == 0)
   {
      m_cat.set(NULL);

      typedef void (*destroyFunc_t)();
      destroyFunc_t f = (destroyFunc_t)m_unloadFunc;
      f();
      m_unloadFunc = NULL;

      ::FreeLibrary((HMODULE)m_dllPtr);
      m_dllPtr = NULL;
   }
}

libStub::libStub()
: m_refCnt(0)
, m_unloadFunc(NULL)
, m_dllPtr(NULL)
{
}

libRef::libRef()
: m_pPtr(&libStub::get())
{
   m_pPtr->addref();
}

libRef::~libRef()
{
   m_pPtr->release();
}

libRef::libRef(const libRef& source)
: m_pPtr(source.m_pPtr)
{
   m_pPtr->addref();
}

libRef& libRef::operator=(const libRef& source)
{
   source.m_pPtr->addref();
   m_pPtr->release();
   m_pPtr = source.m_pPtr;
   return *this;
}

staticModuleServer& staticModuleServer::get()
{
   static staticModuleServer the;
   return the;
}

void staticModuleServer::add(tcatbin::iTypeServer& t)
{
   m_types.push_back(&t);
}

size_t staticModuleServer::getNumTypes() const
{
   return m_types.size();
}

tcatbin::iTypeServer& staticModuleServer::getIthType(size_t i)
{
   return *m_types[i];
}

} // namespace tcat
