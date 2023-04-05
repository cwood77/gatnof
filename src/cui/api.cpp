#include "../cmn/service.hpp"
#include "../tcatlib/api.hpp"
#include "api.hpp"
#include "pen.hpp"
#include <sstream>
#include <stdexcept>

namespace cui { 

void control::initialize(pnt p, size_t l, size_t h)
{
   m_p = p;
   m_l = l;
   m_h = h;

   erase();
}

void control::userInitialize(std::function<void(void)> f)
{
   if(m_userInit) return;
   m_userInit = true;
   f();
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

   m_cache = v;
}

void stringControl::update(const std::string& v)
{
   if(v != m_cache)
      redraw(v);
}

intControl::intControl()
: m_pFmt(new maxValueIntFormatter())
{
}

void intControl::redraw(int v)
{
   tcat::typePtr<cmn::serviceManager> svcMan;
   auto& str = svcMan->demand<pen::object>().str();

   // clear
   erase();

   // write
   str << pen::moveTo(getLoc());
   formatText(str);
   str << m_pFmt->formatValue(v,getLength());

   m_cache = v;
}

void intControl::update(int v)
{
   if(v != m_cache)
      redraw(v);
}

std::string maxValueIntFormatter::formatValue(int v, size_t l) const
{
   // XXX
   // 99+
   //
   // a 3 digit display has a max value of 2 9's
   // >= 100 means display the max value
   // l == 1 is illegal because there's no room for the +

   int maxValue = 10;
   for(size_t i=0;i<(l-2);i++)
      maxValue *= 10;

   std::stringstream stream;

   if(v >= maxValue)
   {
      // sad path: print the max value
      stream << (maxValue-1) << "+";
   }
   else
   {
      // happy path; just print the real value
      stream << v;
   }

   return stream.str();
}

std::string bracketedIntFormatter::formatValue(int v, size_t l) const
{
   auto num = m_pNext->formatValue(v,l-2);
   size_t nPad = l - num.length();

   std::stringstream stream;
   stream << "[" << std::string(nPad,' ') << num << "]";
   return stream.str();
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
