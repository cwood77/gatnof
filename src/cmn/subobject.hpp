#ifndef ___cmn_subobject___
#define ___cmn_subobject___

#include <stdexcept>

namespace console { class iLog; }
namespace sst { class dict; }

namespace cmn {

class iSubobject {
public:
   virtual void tie(sst::dict& c, console::iLog& l) = 0;
};

class subobject : public virtual iSubobject {
public:
   virtual void tie(sst::dict& c, console::iLog& l) { m_pConfig = &c; m_pLog = &l; }

protected:
   subobject() : m_pConfig(NULL), m_pLog(NULL) {}

   sst::dict& config();
   console::iLog& log();

   template<class T>
   T& tie(T& o) { o.tie(config(),log()); return o; }

   template<class T>
   T& tie() { return tie(*new T()); }

private:
   sst::dict *m_pConfig;
   console::iLog *m_pLog;
};

#include "subobject.ipp"

} // namespace cmn

#endif // ___cmn_subobject___
