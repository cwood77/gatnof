#define WIN32_LEAN_AND_MEAN
#include "ani.hpp"
#include "pen.hpp"
#include <windows.h>

namespace ani {
namespace {

class canvasAdapter : public iCanvas {
public:
   canvasAdapter(iCanvas& c, size_t o) : m_c(c), m_o(o) {}

   virtual frame& getFrame(size_t i) { return m_c.getFrame(i+m_o); }

private:
   iCanvas& m_c;
   size_t m_o;
};

} // anonymous namespace

void delay::sleep() { ::Sleep(nMSec); }

void frame::run(pen::object& pn)
{
   for(auto it=m_sketches.begin();it!=m_sketches.end();++it)
      (*it)(pn);
}

void flipbook::run(pen::object& pn)
{
   bool first = true;
   for(auto it=m_f.begin();it!=m_f.end();++it)
   {
      if(first)
         first = false;
      else
         m_d.sleep();
      it->second.run(pn);
   }
}

size_t flipbook::size() const
{
   if(m_f.size())
      return (--m_f.end())->first + 1;
   else
      return 0;
}

frame& flipbook::getFrame(size_t i)
{
   while(m_f.size() <= i)
   {
      size_t next = m_f.size();
      m_f[next];
   }
   return m_f[i];
}

void sequencer::simultaneous(const std::vector<std::function<void(iCanvas&)> >& a)
{
   canvasAdapter ad(m_f,m_f.size());

   for(auto it=a.begin();it!=a.end();++it)
      (*it)(ad);
}

// =======================================================================================
//            ARTISTS

void prim::lineLeftToRight(iCanvas& c, cui::pnt p, size_t l)
{
   size_t iFrame=0;
   size_t iFrame2=0;

   // top
   for(size_t i=0;i<l;i++)
   {
      c.getFrame(iFrame2).add([=](auto& pObj)
      {
         pObj.str()
            << pen::moveTo(cui::pnt(p.x+i,p.y))
            << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kYellow)
            << "-";
      });
      iFrame++;
      if(iFrame % 2 == 0)
         iFrame2++;
   }
}

void prim::lineRightToLeft(iCanvas& c, cui::pnt p, size_t l)
{
   size_t iFrame=0;
   size_t iFrame2=0;

   // bottom
   for(size_t i=l-1;;i--)
   {
      c.getFrame(iFrame2).add([=](auto& pObj)
      {
         pObj.str()
            << pen::moveTo(cui::pnt(p.x+i,p.y))
            << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kYellow)
            << "-";
      });
      iFrame++;
      if(iFrame % 2 == 0)
         iFrame2++;
      if(i==0)
         break;
   }
}

void attendance::colorBox(iCanvas& c, size_t i)
{
   c.getFrame(0).add([=](auto& pObj)
   {
      pObj.str() << pen::fgcol(pen::kWhite,true) << pen::bgcol(pen::kGreen,true);
      for(size_t j=0;j<3;j++)
         pObj.str() << pen::moveTo(cui::pnt(24+15*(i-1),10+j)) << std::string(5,' ');
      pObj.str() << pen::moveTo(cui::pnt(24+15*(i-1)+2,10+1)) << i;
   });
}

void attendance::drawConnection(iCanvas& c, size_t i)
{
   size_t iFrame=0;

   cui::pnt p(27+15*(i-1)+2,11);

   for(size_t i=0;i<10;i++)
   {
      c.getFrame(iFrame++).add([=](auto& pObj)
      {
         pObj.str() << pen::fgcol(pen::kWhite,true) << pen::bgcol(pen::kGreen/*,true*/);
         pObj.str() << pen::moveTo(cui::pnt(p.x+i,p.y)) << " ";
      });
   }
}

void outliner::outline(iCanvas& c)
{
   cui::pnt p = m_c.getLoc();
   size_t iFrame=0;

   // tl corner
   c.getFrame(iFrame++).add([=](auto& pObj)
   {
      pObj.str()
         << pen::moveTo(p)
         << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
         << "+";
   });

   // top
   for(size_t i=0;i<m_c.getLength()-2;i++)
   {
      c.getFrame(iFrame++).add([=](auto& pObj)
      {
         pObj.str()
            << pen::moveTo(cui::pnt(p.x+1+i,p.y))
            << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
            << "-";
      });
   }

   // tr corner
   p = cui::pnt(p.x+m_c.getLength()-1,p.y);
   c.getFrame(iFrame++).add([=](auto& pObj)
   {
      pObj.str()
         << pen::moveTo(p)
         << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
         << "+";
   });

   // right
   for(size_t i=0;i<m_c.getHeight()-2;i++)
   {
      c.getFrame(iFrame++).add([=](auto& pObj)
      {
         pObj.str()
            << pen::moveTo(cui::pnt(p.x,p.y+1+i))
            << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
            << "|";
      });
   }

   // br corner
   p = m_c.getLoc();
   p = cui::pnt(p.x+m_c.getLength()-1,p.y+m_c.getHeight()-1);
   c.getFrame(iFrame++).add([=](auto& pObj)
   {
      pObj.str()
         << pen::moveTo(p)
         << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
         << "+";
   });

   // bottom
   p = m_c.getLoc();
   p = cui::pnt(p.x+1,p.y+m_c.getHeight()-1);
   for(size_t i=m_c.getLength()-3;;i--)
   {
      c.getFrame(iFrame++).add([=](auto& pObj)
      {
         pObj.str()
            << pen::moveTo(cui::pnt(p.x+i,p.y))
            << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
            << "-";
      });
      if(i==0)
         break;
   }

   // bl corner
   p = m_c.getLoc();
   p = cui::pnt(p.x,p.y+m_c.getHeight()-1);
   c.getFrame(iFrame++).add([=](auto& pObj)
   {
      pObj.str()
         << pen::moveTo(p)
         << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
         << "+";
   });

   // left
   p = m_c.getLoc();
   p = cui::pnt(p.x,p.y+m_c.getHeight()-2);
   for(size_t i=m_c.getHeight()-3;;i--)
   {
      c.getFrame(iFrame++).add([=](auto& pObj)
      {
         pObj.str()
            << pen::moveTo(cui::pnt(p.x,p.y+i))
            << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
            << "|";
      });
      if(i==0)
         break;
   }
}

void blinker::blink(iCanvas& c)
{
}

} // namespace ani
