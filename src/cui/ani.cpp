#define WIN32_LEAN_AND_MEAN
#include "ani.hpp"
#include "pen.hpp"
#include <conio.h>
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

void delay::sleep()
{
   if(m_nCnt == 0)
   {
      ::Sleep(nMSec);
      m_nCnt = nSkip;
   }
   else
      m_nCnt--;
}

void delayTweakKeystrokeMonitor::stop()
{
   m_stopSignal.set();
   m_tc.join();
}

void delayTweakKeystrokeMonitor::run()
{
   while(true)
   {
      if(m_stopSignal.isSet())
         return;
      if(::kbhit())
      {
         char c = ::getch();
         if(c == '+')
         {
            if(m_d.nMSec == 0 && m_d.nSkip == 0)
            {
               m_d.nMSec = 1;
               m_d.nSkip = 3;
            }
            else if(m_d.nSkip != 0)
               m_d.nSkip--;
            else
               m_d.nMSec++;
            m_wasTweaked = true;
         }
         else if(c == '-')
         {
            if(m_d.nMSec == 0 && m_d.nSkip == 0)
               ;
            else if(m_d.nMSec > 1)
               m_d.nMSec--;
            else if(m_d.nSkip < 3)
               m_d.nSkip++;
            else
            {
               m_d.nMSec = 0;
               m_d.nSkip = 0;
            }
            m_wasTweaked = true;
         }
         else if(c == '0')
         {
            m_d.nMSec = 0;
            m_d.nSkip = 0;
            m_wasTweaked = true;
         }
         else
         {
            auto it = m_extra.find(c);
            if(it != m_extra.end())
               it->second();
            else
               ; // swallow other keystrokes and ignore
         }
      }
   }
}

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

void prim::box(frame& f, cui::pnt ul, size_t l, size_t h, pen::colors bgcol)
{
   if(l < 3)
      throw std::runtime_error("l too small");
   if(h < 3)
      throw std::runtime_error("h too small");
   f.add([=](auto& pObj)
   {
      // top
      pObj.str()
         << pen::moveTo(ul)
         << pen::bgcol(bgcol)
         << " ";
      for(size_t i=0;i<l-2;i++)
         pObj.str() << " ";
      pObj.str() << " ";

      // bottom
      pObj.str()
         << pen::moveTo(cui::pnt(ul.x,ul.y+h-1))
         << pen::bgcol(bgcol)
         << " ";
      for(size_t i=0;i<l-2;i++)
         pObj.str() << " ";
      pObj.str() << " ";

      // left
      pObj.str()
         << pen::moveTo(cui::pnt(ul.x,ul.y+1))
         << pen::bgcol(bgcol)
         << " ";

      // right
      pObj.str()
         << pen::moveTo(cui::pnt(ul.x+l-1,ul.y+1))
         << pen::bgcol(bgcol)
         << " ";
   });
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

void outliner::outline(cui::pnt P, size_t l, size_t h, iCanvas& c)
{
   cui::pnt p = P;
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
   for(size_t i=0;i<l-2;i++)
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
   p = cui::pnt(p.x+l-1,p.y);
   c.getFrame(iFrame++).add([=](auto& pObj)
   {
      pObj.str()
         << pen::moveTo(p)
         << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
         << "+";
   });

   // right
   for(size_t i=0;i<h-2;i++)
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
   p = P;
   p = cui::pnt(p.x+l-1,p.y+h-1);
   c.getFrame(iFrame++).add([=](auto& pObj)
   {
      pObj.str()
         << pen::moveTo(p)
         << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
         << "+";
   });

   // bottom
   p = P;
   p = cui::pnt(p.x+1,p.y+h-1);
   for(size_t i=l-3;;i--)
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
   p = P;
   p = cui::pnt(p.x,p.y+h-1);
   c.getFrame(iFrame++).add([=](auto& pObj)
   {
      pObj.str()
         << pen::moveTo(p)
         << pen::fgcol(pen::kYellow,true) << pen::bgcol(pen::kBlue)
         << "+";
   });

   // left
   p = P;
   p = cui::pnt(p.x,p.y+h-2);
   for(size_t i=h-3;;i--)
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

} // namespace ani
