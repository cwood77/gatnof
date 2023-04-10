#define WIN32_LEAN_AND_MEAN
#include "../../gen/screen/screen.summonResults.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../db/api.hpp"
#include "../file/api.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include <conio.h>
#include <memory>
#include <windows.h>

namespace {

class logic : private summonResults_screen, public cui::iLogic {
public:
   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool)
   {
      tcat::typePtr<db::iDict> dbDict;
      tcat::typePtr<cmn::serviceManager> svcMan;
      auto& ch = svcMan->demand<net::iChannel>();

      std::unique_ptr<sst::dict> pReply(ch.recvSst());

      render();

      bool leftRight = false;
      size_t iRow = 0;

      auto& pulls = (*pReply)["pulls"].as<sst::array>();
      for(size_t i=0;i<pulls.size();i++)
      {
         if(i)
            ::Sleep(40);
         auto& noob = pulls[i].as<sst::dict>();
         drawDraw(leftRight,iRow,*dbDict,
            noob["type"].as<sst::mint>().get(),
            noob["shard"].as<sst::tf>().get());
         pickNewCell(leftRight,iRow);
      }

      m_anyKey.redraw("<any key to dismiss>");

      ::getch();
   }

private:
   void drawDraw(bool leftRight, size_t iRow, db::iDict& db, size_t charId, bool isShard)
   {
      auto& Char = db.findChar(charId);
      if(leftRight)
         drawDraw(m_rList[iRow].rRarity,m_rList[iRow].rName,m_rList[iRow].rShards,
            Char,isShard);
      else
         drawDraw(m_lList[iRow].lRarity,m_lList[iRow].lName,m_lList[iRow].lShards,
            Char,isShard);
   }

   template<class A, class B, class C>
   void drawDraw(A& rarityCtl, B& nameCtl, C& shardCtl, const db::staticChar& Char, bool isShard)
   {
      switch(Char.rarity)
      {
         case db::kR:
            rarityCtl.redraw("  R");
            break;
         case db::kSr:
            rarityCtl.redraw(" SR");
            break;
         case db::kSsr:
            rarityCtl.redraw("SSR");
            break;
         case db::kUr:
            rarityCtl.redraw(" UR");
            break;
      }

      nameCtl.redraw(Char.name);

      if(isShard)
         shardCtl.redraw("(shards)");
   }

   void pickNewCell(bool& leftRight, size_t& iRow)
   {
      ++iRow;

      if(iRow < m_lList.size())
         return;

      // crap, out of rows

      if(leftRight == false)
      {
         // left
         leftRight = true;
         iRow = 0;
         m_rList.erase();
      }
      else
      {
         // right
         leftRight = false;
         iRow = 0;
         m_lList.erase();
      }
   }
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("summonResults") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
