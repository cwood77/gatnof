#ifndef ___tcatbin_tables___
#define ___tcatbin_tables___

#include <map>
#include <set>
#include <string>

namespace tcatbin {

class iModuleServer;
class iTypeServer;

// holds all server DLLs in memory
class libTable {
public:
   ~libTable();

   void add(void* pLib);

private:
   std::set<void*> m_libs;
};

class instTable {
public:
   ~instTable();
   void *create(iTypeServer& svr);
   void release(void *pPtr);
   void releaseSingletons();

private:
   std::map<void*,iTypeServer*> m_ptrs;
   std::map<iTypeServer*,void*> m_singletons;
};

} // namespace tcatbin

#endif // ___tcatbin_tables___
