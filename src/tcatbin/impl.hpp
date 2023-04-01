#ifndef ___tcatbin_impl___
#define ___tcatbin_impl___

#include "api.hpp"
#include "metadata.hpp"
#include "tables.hpp"

namespace tcatbin {

// surveys and loads client binaries
class catalog : public iCatalog {
public:
   catalog();

   virtual void *createSingleType(const char *pTypeName);
   virtual void *createMultipleTypes(const char *pTypeName, size_t& n);
   virtual void releaseType(void *pPtr);
   virtual void releaseMultipleTypes(void *pPtr, size_t n);

private:
   libTable m_libs;
   catalogMetadata m_metadata;
   instTable m_inst;
};

// creates/destroys the catalog
class catalogRef {
public:
   static catalogRef& get();

   iCatalog& addref();
   void release();

   void diag();

private:
   catalogRef();

   int32_t m_refCnt;
   iCatalog *m_pInstance;
};

} // namespace tcatbin

#endif // ___tcatbin_impl___

