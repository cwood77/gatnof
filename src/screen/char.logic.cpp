#include "../../gen/screen/screen.char.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/ani.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../db/api.hpp"
#include "../file/api.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>
#include <memory>

namespace {

class iCharSorter {
public:
   virtual void rebuild(db::iDict& db, sst::dict& acct) = 0;
   virtual void setPg(size_t& pg) = 0;
   virtual void redraw(std::function<void(db::Char&,int)> f) = 0;
};

class rarityCharSorter : public iCharSorter {
public:
   explicit rarityCharSorter(size_t nRows) : m_nRows(nRows) {}
   ~rarityCharSorter() { free(); }

   virtual void rebuild(db::iDict& db, sst::dict& acct)
   {
      free();
      auto& m = acct["chars"].as<sst::dict>().asMap();
      for(auto it=m.begin();it!=m.end();++it)
         m_set.insert(new db::Char(db,it->second->as<sst::dict>(),0));
   }

   virtual void setPg(size_t& pg)
   {
      m_pg = pg;
   }

   virtual void redraw(std::function<void(db::Char&,int)> f)
   {
      size_t nSkip = m_pg ? (m_pg - 1) * m_nRows : 0;
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

   class order {
   public:
      typedef db::Char *type_t;

      bool operator()(const type_t& lhs, const type_t& rhs) const
      {
         return lhs < rhs;
      }
   };

   std::set<db::Char*,order> m_set;
   const size_t m_nRows;
   size_t m_pg;
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

      const char *gSelDisp[] = { "detail ","line-up" };
      int selMode = 0;

      const char *gSortDisp[] = { "rarity" };
      int sortMode = 0;

      size_t pg = 0;

      // whole screen re-draw
      // note: this is here so I can read table size
      render();

      rarityCharSorter rSort(m_table.size());
      iCharSorter *pSort = &rSort;

      while(true)
      {
         // static controls

         // dynamic controls
         pSort->rebuild(*dbDict,(*acct));
         pSort->setPg(pg);
         pSort->redraw([&](auto& ch, int idx){
            // draw each char

            // TODO: in team

            static const char *gRarities[] = { "  R", " SR", "SSR", " UR" };
            m_table[idx].rarity.redraw(gRarities[ch.rarity()]);

            // TODO: star

            m_table[idx].lvl.redraw(ch.getLevel());

            m_table[idx].name.redraw(ch.name());

            m_table[idx].atk.redraw(ch.atk(false));
            m_table[idx].def.redraw(ch.def());
            m_table[idx].agil.redraw(ch.agil());

            static const char *gElements[] = { "water", "fire ", "earth" };
            m_table[idx].element.redraw(gElements[ch.element()]);

            /*std::stringstream combinedCaste;
            combinedCaste << ch.caste << ";" << ch.subcaste;
            m_table[idx].caste.redraw(combinedCaste.str());*/
         });
         m_selectModeDsp.redraw(gSelDisp[selMode]);
         m_sortModeDsp.redraw(gSortDisp[sortMode]);

         // handle user input
         cui::buttonHandler handler(m_error);
         handler.add(m_backBtn,[&](bool& stop){ stop = true; });
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
