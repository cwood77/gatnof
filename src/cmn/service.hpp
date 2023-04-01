#ifndef ___cmn_service___
#define ___cmn_service___

#include <map>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace cmn {

// instantiation of this guy is left to you, which controls the lifetime/scope

class serviceManager {
public:
   template<class T> void attach(T& intf, const std::string& id = "")
   {
      void*& pPtr = m_map[makeId<T>(id)];
      if(pPtr)
         throw std::runtime_error("duplicate attach");
      pPtr = &intf;
   }

   template<class T> void detach(T& intf, const std::string& id = "")
   {
      void*& pPtr = m_map[makeId<T>(id)];
      if(!pPtr)
         throw std::runtime_error("service not found");
      pPtr = NULL;
   }

   template<class T> T& demand(const std::string& id = "")
   {
      void *pPtr = m_map[makeId<T>(id)];
      if(!pPtr)
         throw std::runtime_error("service not found");
      return *reinterpret_cast<T*>(pPtr);
   }

private:
   template<class T> std::string makeId(const std::string& id)
   {
      return id + ":" + typeid(T).name();
   }

   std::map<std::string,void*> m_map;
};

template<class T>
class autoService {
public:
   autoService(serviceManager& man, T& svc, const std::string& id = "")
   : m_pSvc(&svc)
   , m_id(id)
   , m_pSMan(&man)
   {
      m_pSMan->attach<T>(*m_pSvc,m_id);
   }

   ~autoService()
   {
      m_pSMan->detach<T>(*m_pSvc,m_id);
   }

private:
   T *m_pSvc;
   std::string m_id;
   serviceManager *m_pSMan;
};

} // namespace cmn

#endif // ___cmn_service___
