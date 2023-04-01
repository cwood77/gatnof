#include "../cmn/misc.hpp"
#include "tables.hpp"
#include "api.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tcatbin {

libTable::~libTable()
{
   std::set<void*>::iterator it = m_libs.begin();
   for(;it!=m_libs.end();++it)
      ::FreeLibrary((HMODULE)*it);
}

void libTable::add(void* pLib)
{
   m_libs.insert(pLib);
}

instTable::~instTable()
{
   releaseSingletons();
   if(m_ptrs.size())
      ::printf("[tcatbin::instTable] leaking %lld instances...?\n",m_ptrs.size());
   if(m_singletons.size())
      ::printf("[tcatbin::instTable] leaking %lld singletons...?\n",m_ptrs.size());
}

void *instTable::create(iTypeServer& svr)
{
   const bool isSingleton = svr.getFlags() & iTypeServer::kSingleton;
   if(isSingleton)
   {
      void*& pInst = m_singletons[&svr];
      if(!pInst)
      {
         pInst = svr.createType();
         m_ptrs[pInst] = &svr;
      }
      return pInst;
   }
   else
   {
      void *pPtr = svr.createType();
      m_ptrs[pPtr] = &svr;
      return pPtr;
   }
}

void instTable::release(void *pPtr)
{
   iTypeServer& svr = *m_ptrs[pPtr];
   const bool isSingleton = svr.getFlags() & iTypeServer::kSingleton;
   if(isSingleton)
   {
      // ignored for singletons; they live until shutdown
   }
   else
   {
      svr.releaseType(pPtr);
      m_ptrs.erase(pPtr);
   }
}

void instTable::releaseSingletons()
{
   cmn::runawayLoopCheck chk;
   while(m_singletons.size())
   {
      chk.sanityCheck();

      auto it = m_singletons.begin();
      auto& svr = *it->first;
      auto *pPtr = it->second;

      m_ptrs.erase(pPtr);
      m_singletons.erase(it);

      svr.releaseType(pPtr);
   }
}

} // namespace tcatbin
