#include "../cmn/service.hpp"
#include "../tcatlib/api.hpp"
#include "api.hpp"
#include "pen.hpp"
#include <stdexcept>

namespace cui { 

void control::initialize(pnt p, size_t l, size_t h)
{
   m_p = p;
   m_l = l;
   m_h = h;

   erase();
}

void control::erase()
{
   tcat::typePtr<cmn::serviceManager> svcMan;
   auto& str = svcMan->demand<pen::object>().str();

   // clear
   for(size_t y=0;y<m_h;y++)
   {
      str << pen::moveTo(cui::pnt(m_p.x,m_p.y+y));
      formatText(str);
      str << std::string(m_l,' ');
   }
}

void control::formatText(std::ostream& o)
{
   if(m_i == 1)
      formatText1(o);
   else if(m_i == 2)
      formatText2(o);
}

void stringControl::redraw(const std::string& v)
{
   tcat::typePtr<cmn::serviceManager> svcMan;
   auto& str = svcMan->demand<pen::object>().str();

   // clear
   erase();

   // write
   str << pen::moveTo(getLoc());
   formatText(str);
   str << v;

   m_str = v;
}

void stringControl::update(const std::string& v)
{
   if(v != m_str)
      redraw(v);
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
