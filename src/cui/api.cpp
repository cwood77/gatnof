#include "../cmn/service.hpp"
#include "../tcatlib/api.hpp"
#include "api.hpp"
#include "pen.hpp"
#include <stdexcept>

namespace cui { 

void control::initialize(pnt p, size_t l)
{
   m_p = p;
   m_l = l;
   update(std::string(l,' '));
}

void control::update(const std::string& v)
{
   tcat::typePtr<cmn::serviceManager> svcMan;
   auto& str = svcMan->demand<pen::object>().str();

   // clear
   str << pen::moveTo(m_p);
   formatText(str);
   str << std::string(m_l,' ');

   // write
   str << pen::moveTo(m_p);
   formatText(str);
   str << v;

   m_str = v;
}

void control::access(std::function<void(std::string&)> f)
{
   std::string previous = m_str;
   f(m_str);
   if(previous != m_str)
      update(m_str);
}

iObject& basicScreen::_demand(const std::string& id)
{
   iObject*& pObj = m_map[id];
   if(!pObj)
      throw std::runtime_error("undefined object");
   return *pObj;
}

void basicScreen::publishObject(const std::string& id, iObject& o)
{
   iObject*& pObj = m_map[id];
   if(pObj)
      throw std::runtime_error("multiply defined object");
   pObj = &o;
}

} // namespace cui
