#include "impl.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tcatbin {

iCatalog& iCatalog::create()
{
   return catalogRef::get().addref();
}

void iCatalog::destroy()
{
   catalogRef::get().release();
}

catalog::catalog()
{
   fileReflector fileR(m_metadata,m_libs);
   folderReflector folderR(fileR);

   char path[MAX_PATH];
   ::GetModuleFileNameA(NULL,path,MAX_PATH);
   std::string folderPath = path;
   folderPath += "\\..";
   folderR.reflectFolder(folderPath);
}

void *catalog::createSingleType(const char *pTypeName)
{
   return m_inst.create(m_metadata.demandOne(pTypeName));
}

void *catalog::createMultipleTypes(const char *pTypeName, size_t& n)
{
   std::set<iTypeServer*> types = m_metadata.getAll(pTypeName);

   size_t i=0;
   void **pBlock = new void*[types.size()];
   std::set<iTypeServer*>::iterator it = types.begin();
   for(i=0;it!=types.end();++it,i++)
      pBlock[i] = m_inst.create(**it);

   n = i;
   return pBlock;
}

void catalog::releaseType(void *pPtr)
{
   m_inst.release(pPtr);
}

void catalog::releaseMultipleTypes(void *pPtr, size_t n)
{
   void **pBlock = (void**)pPtr;

   for(size_t i=0;i<n;i++)
      releaseType(pBlock[i]);

   delete [] pBlock;
}

catalogRef& catalogRef::get()
{
   static catalogRef the;
   return the;
}

iCatalog& catalogRef::addref()
{
   if(::InterlockedIncrement((LONG*)&m_refCnt) == 1)
      m_pInstance = new catalog();

   return *m_pInstance;
}

void catalogRef::release()
{
   if(::InterlockedDecrement((LONG*)&m_refCnt) == 0)
   {
      delete m_pInstance;
      m_pInstance = NULL;
   }
}

catalogRef::catalogRef()
: m_refCnt(0)
, m_pInstance(NULL)
{
}

void catalogRef::diag()
{
   if(!m_pInstance)
      return;

   ::printf("****** BUG ******\n");
   ::printf("outstanding (i.e. leaked) refcnt on tcatbin upon DLL unload\n");
   ::printf("  catalog refcnt = %d\n",m_refCnt);
   ::printf("  catalog ptr = %llu\n",(size_t)m_pInstance);
}

} // namespace tcatbin

BOOL WINAPI DllMain(HINSTANCE, DWORD d, LPVOID)
{
   if(d == DLL_PROCESS_DETACH)
      tcatbin::catalogRef::get().diag();

   return TRUE;
}
