#include "../../gen/screen/screen.char.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../db/api.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include <memory>

namespace {

class iCharSorter {
public:
   virtual void rebuild(db::iDict& db, sst::dict& acct) = 0;
   virtual void setPg(long& pg) = 0;
   virtual void redraw(std::function<void(db::Char&,int)> f) = 0;
};

class rarityOrder {
public:
   typedef db::Char *type_t;

   bool operator()(const type_t& lhs, const type_t& rhs) const
   {
      // rarity
      auto lR = lhs->rarity();
      auto rR = rhs->rarity();
      if(lR != rR)
         return lR > rR;

      // star
      auto lS = lhs->getStars();
      auto rS = rhs->getStars();
      if(lS != rS)
         return lS > rS;

      // level
      auto lL = lhs->getLevel();
      auto rL = rhs->getLevel();
      if(lL != rL)
         return lL > rL;

      // atk
      auto lA = lhs->atk(false);
      auto rA = rhs->atk(false);
      if(lA != rA)
         return lA > rA;

      // name
      return lhs->name() < rhs->name();
   }
};

class casteOrder {
public:
   typedef db::Char *type_t;

   bool operator()(const type_t& lhs, const type_t& rhs) const
   {
      // rarity
      auto lC = lhs->caste() + "; " + lhs->subcaste();
      auto rC = rhs->caste() + "; " + rhs->subcaste();
      if(lC != rC)
         return lC < rC;

      // name
      return lhs->name() < rhs->name();
   }
};

template<class T>
class charSorter : public iCharSorter {
public:
   explicit charSorter(size_t nRows) : m_nRows(nRows) {}
   ~charSorter() { free(); }

   virtual void rebuild(db::iDict& db, sst::dict& acct)
   {
      free();
      auto& m = acct["chars"].as<sst::dict>().asMap();
      for(auto it=m.begin();it!=m.end();++it)
         m_set.insert(new db::Char(db,it->second->as<sst::dict>(),0));
   }

   virtual void setPg(long& pg)
   {
      if(pg < 0)
         pg = 0;

      long nPgs = m_set.size() / m_nRows;
      if(!nPgs || (m_set.size() % m_nRows))
         nPgs++;

      if(pg >= nPgs)
         pg = nPgs-1;

      m_pg = pg;
   }

   virtual void redraw(std::function<void(db::Char&,int)> f)
   {
      size_t nSkip = m_pg ? m_pg * m_nRows : 0;
      size_t nIdx = 0;

      for(auto *pC : m_set)
      {
         if(nSkip)
         {
            --nSkip;
            continue;
         }

         if(nIdx >= m_nRows)
            break;

         f(*pC,nIdx++);
      }
   }

private:
   void free()
   {
      for(auto *pC : m_set)
         delete pC;
      m_set.clear();
   }

   std::set<db::Char*,T> m_set;
   const size_t m_nRows;
   long m_pg;
};

class iCharSelector {
public:
   virtual void run(db::Char *c, cui::stringControl& error, bool& stop) = 0;
};

class inTeamCharSelector : public iCharSelector {
public:
   virtual void run(db::Char *c, cui::stringControl& error, bool& stop)
   {
      if(c == NULL)
      {
         error.redraw("No character selected");
         return;
      }

      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();
      auto& ch = svcMan->demand<net::iChannel>();

      ch.sendString("toggleInTeam");
      std::stringstream stream;
      stream << c->getType();
      ch.sendString(stream.str());

      auto resp = ch.recvString();

      if(!resp.empty())
         error.redraw(resp);
      else
      {
         acct.reset(ch.recvSst());
         stop = true;
      }
   }
};

class detailCharSelector : public iCharSelector {
public:
   virtual void run(db::Char *c, cui::stringControl& error, bool& stop)
   {
      if(c == NULL)
      {
         error.redraw("No character selected");
         return;
      }

      tcat::typePtr<cmn::serviceManager> svcMan;
      size_t cType = c->getType();
      cmn::autoService<size_t> _char(*svcMan,cType,"selectedChar");
      tcat::typePtr<cui::iFactory> sFac;
      cmn::autoReleasePtr<cui::iLogic> pL(&sFac->create<cui::iLogic>("charDetail"));
      pL->run();

      stop = true;
   }
};

class logic : private char_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool interactive)
   {
      tcat::typePtr<db::iDict> dbDict;
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& acct = svcMan->demand<std::unique_ptr<sst::dict> >();

      const char *gSelDisp[] = { "line-up", "detail " };
      int selMode = 0;

      const char *gSortDisp[] = { "rarity", "caste " };
      int sortMode = 0;

      long pg = 0;

      // whole screen re-draw
      // note: this is here so I can read table size
      render();

      charSorter<rarityOrder> rSort(m_table.size());
      charSorter<casteOrder> sSort(m_table.size());
      iCharSorter *pSort = &rSort;

      inTeamCharSelector iTSelect;
      detailCharSelector dSelect;
      iCharSelector *pSelect = &iTSelect;

      while(true)
      {
         // build a look-up for team membership
         std::set<size_t> inTeam;
         {
            auto& lineUp = (*acct)["line-up"].as<sst::array>();
            for(size_t i=0;i<lineUp.size();i++)
               inTeam.insert(lineUp[i].as<sst::mint>().get());
         }

         // main table
         std::vector<db::Char*> visibleChars;
         visibleChars.resize(m_table.size());
         pSort->rebuild(*dbDict,(*acct));
         pSort->setPg(pg);
         pSort->redraw([&](auto& ch, int idx){

            // draw each char

            bool isIn = (inTeam.find(ch.getType()) != inTeam.end());
            if(isIn)
               m_table[idx].inTeam.redraw("*");

            m_table[idx].rarity.redraw(db::fmtRaritiesFixedWidth(ch.rarity()));

            m_table[idx].star.redraw(db::fmtStarsFixedWidth(ch.getStars()));

            m_table[idx].lvl.redraw(ch.getLevel());

            m_table[idx].name.redraw(ch.name());

            m_table[idx].atk.redraw(ch.atk(false));
            m_table[idx].def.redraw(ch.def());
            m_table[idx].agil.redraw(ch.agil());

            m_table[idx].element.redraw(db::fmtElementsFixedWidth(ch.element()));

            std::stringstream combinedCaste;
            combinedCaste << ch.caste() << "; " << ch.subcaste();
            m_table[idx].caste.redraw(combinedCaste.str());

            visibleChars[idx] = &ch;
         });
         m_selectModeDsp.redraw(gSelDisp[selMode]);
         m_sortModeDsp.redraw(gSortDisp[sortMode]);

         // footer
         if((*acct)["line-up-bonus"].as<sst::mint>().get())
         {
            m_bonusDisp.setFormatMode(2);
            m_bonusDisp.redraw("bonus: 5 members of same caste");
         }
         else
         {
            m_bonusDisp.setFormatMode(1);
            m_bonusDisp.erase();
         }
         m_teamCnt.redraw(inTeam.size());

         // handle user input
         cui::buttonHandler handler(m_error);
         handler.addCustom('0',[&](bool& stop){ pSelect->run(visibleChars[0],m_error,stop); });
         handler.addCustom('1',[&](bool& stop){ pSelect->run(visibleChars[1],m_error,stop); });
         handler.addCustom('2',[&](bool& stop){ pSelect->run(visibleChars[2],m_error,stop); });
         handler.addCustom('3',[&](bool& stop){ pSelect->run(visibleChars[3],m_error,stop); });
         handler.addCustom('4',[&](bool& stop){ pSelect->run(visibleChars[4],m_error,stop); });
         handler.addCustom('5',[&](bool& stop){ pSelect->run(visibleChars[5],m_error,stop); });
         handler.addCustom('6',[&](bool& stop){ pSelect->run(visibleChars[6],m_error,stop); });
         handler.addCustom('7',[&](bool& stop){ pSelect->run(visibleChars[7],m_error,stop); });
         handler.addCustom('8',[&](bool& stop){ pSelect->run(visibleChars[8],m_error,stop); });
         handler.addCustom('9',[&](bool& stop){ pSelect->run(visibleChars[9],m_error,stop); });
         handler.addCustom('A',[&](bool& stop){ pSelect->run(visibleChars[10],m_error,stop); });
         handler.addCustom('B',[&](bool& stop){ pSelect->run(visibleChars[11],m_error,stop); });
         handler.addCustom('C',[&](bool& stop){ pSelect->run(visibleChars[12],m_error,stop); });
         handler.add(m_backBtn,[&](bool& stop){ stop = true; });
         handler.add(m_upBtn,[&](bool& stop){ pg--; stop = true; });
         handler.add(m_downBtn,[&](bool& stop){ pg++; stop = true; });
         handler.add(m_selectModeBtn,[&](bool& stop)
         {
            selMode++;
            int N = sizeof(gSelDisp) / sizeof(const char *);
            if(selMode >= N)
               selMode = 0;
            pSelect = selMode ? (iCharSelector*)&dSelect : (iCharSelector*)&iTSelect;
            m_selectModeDsp.redraw(gSelDisp[selMode]);
         });
         handler.add(m_sortModeBtn,[&](bool& stop)
         {
            sortMode++;
            int N = sizeof(gSortDisp) / sizeof(const char *);
            if(sortMode >= N)
               sortMode = 0;
            pSort = sortMode ? (iCharSorter*)&sSort : (iCharSorter*)&rSort;
            stop = true;
         });
         auto *ans = handler.run(svcMan->demand<cui::iUserInput>());
         if(ans == &m_backBtn)
            return;

         // whole screen re-draw
         render();
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("char") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
