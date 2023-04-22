#include "../../gen/screen/screen.charDetail.hpp"
#include "../cmn/autoPtr.hpp"
#include "../cmn/service.hpp"
#include "../cui/api.hpp"
#include "../cui/pen.hpp"
#include "../db/api.hpp"
#include "../net/api.hpp"
#include "../tcatlib/api.hpp"
#include <memory>

namespace {

class logic : private charDetail_screen, public cui::iLogic {
public:
   logic()
   : m_acct(m_svcMan->demand<std::unique_ptr<sst::dict> >())
   {
   }

   // required for diamond inheritance :(
   virtual void release() { delete this; }

   virtual void run(bool interactive)
   {
      long pg = 0;

      // setup int formatting
      setupControls();

      // whole screen re-draw
      render();

      // select / sort equip panel

      // draw equip panel
      m_subtableTitle.update(" WEAPON  ");

      while(true)
      {
         // grab the char that's been selected by my parent
         reloadChar();

         // draw main panel form fields
         drawBasicCharFields();
         drawStats();
         drawCurrencies();

         // handle user input
         cui::buttonHandler handler(m_error);
         handler.addCustom('0',[&](bool& stop){  });
         handler.addCustom('1',[&](bool& stop){  });
         handler.addCustom('2',[&](bool& stop){  });
         handler.addCustom('3',[&](bool& stop){  });
         handler.addCustom('4',[&](bool& stop){  });
         handler.addCustom('5',[&](bool& stop){  });
         handler.addCustom('6',[&](bool& stop){  });
         handler.addCustom('7',[&](bool& stop){  });
         handler.addCustom('8',[&](bool& stop){  });
         handler.addCustom('9',[&](bool& stop){  });
         handler.addCustom('A',[&](bool& stop){  });
         handler.addCustom('B',[&](bool& stop){  });
         handler.addCustom('C',[&](bool& stop){  });
         handler.add(m_backBtn,[&](bool& stop){ stop = true; });
         handler.add(m_levelUpBtn,[&](bool& stop)
         {
            // message the server
            // receive updates (or error)
            stop = true; // redraw
         });
         // star up
         handler.add(m_upBtn,[&](bool& stop){ pg--; stop = true; });
         handler.add(m_downBtn,[&](bool& stop){ pg++; stop = true; });
         // equip next
         auto *ans = handler.run(m_svcMan->demand<cui::iUserInput>());
         if(ans == &m_backBtn)
            return;

         // whole screen re-draw
         //render();
         // instead, just clear equip panel
         // erase error?
      }
   }

private:
   void setupControls()
   {
      rightJustifyMaxIntCtl(m_starCnt);
      rightJustifyMaxIntCtl(m_level);

      rightJustifyMaxIntCtl(m_atkBase);
      rightJustifyMaxIntCtl(m_atkEquip);
      rightJustifyMaxIntCtl(m_atkTotal);

      rightJustifyMaxIntCtl(m_defBase);
      rightJustifyMaxIntCtl(m_defEquip);
      rightJustifyMaxIntCtl(m_defTotal);

      rightJustifyMaxIntCtl(m_agilBase);
      rightJustifyMaxIntCtl(m_agilEquip);
      rightJustifyMaxIntCtl(m_agilTotal);

      leftJustifyHugeIntCtl(m_shards);
      leftJustifyHugeIntCtl(m_gold);
   }

   void rightJustifyMaxIntCtl(cui::intControl& c)
   {
      c.setFormatter(
         *new cui::justifyingIntFormatter(/*right*/true,
            *new cui::trustingIntFormatter()));
   }

   void leftJustifyHugeIntCtl(cui::intControl& c)
   {
      c.setFormatter(
         *new cui::justifyingIntFormatter(/*right*/false,
            *new cui::hugeValueIntFormatter()));
   }

   void reloadChar()
   {
      size_t cType = m_svcMan->demand<size_t>("selectedChar");

      std::stringstream sKey;
      sKey << cType;
      auto& overlay = (*m_acct)["chars"].as<sst::dict>()[sKey.str()].as<sst::dict>();

      m_char.reset(new db::Char(*m_dbDict,overlay,0));
   }

   void drawBasicCharFields()
   {
      m_rarity.update(db::fmtRaritiesFixedWidth(m_char->rarity()));
      m_name.update(m_char->name());

      m_element.update(db::fmtElementsFixedWidth(m_char->element()));
      m_caste.update(m_char->caste());
      m_subcaste.update(m_char->subcaste());

      m_starCnt.update(m_char->getStars());
      m_starGlyph.update(db::fmtStarsFixedWidth(m_char->getStars()));
      m_level.update(m_char->getLevel());
   }

   void drawStats()
   {
      m_atkBase.update(m_char->getStat(false,NULL));
      m_atkEquip.update(m_char->getEquip(0) ? m_char->getEquip(0)->quality : 0);
      m_atkTotal.update(m_char->atk(false));

      m_defBase.update(m_char->getStat(false,NULL));
      m_defEquip.update(m_char->getEquip(1) ? m_char->getEquip(1)->quality : 0);
      m_defTotal.update(m_char->def());

      m_agilBase.update(m_char->getStat(false,NULL));
      m_agilEquip.update(m_char->getEquip(2) ? m_char->getEquip(2)->quality : 0);
      m_agilTotal.update(m_char->agil());
   }

   void drawCurrencies()
   {
      std::stringstream sKey;
      sKey << m_char->getType();
      auto& inven = (*m_acct)["items"].as<sst::dict>();
      if(!inven.has(sKey.str()))
         m_shards.update(0);
      else
         m_shards.update(inven[sKey.str()].as<sst::dict>()["amt"].as<sst::mint>().get());

      m_gold.update((*m_acct)["gold"].as<sst::mint>().get());
   }

   tcat::typePtr<cmn::serviceManager> m_svcMan;
   std::unique_ptr<sst::dict>& m_acct;

   tcat::typePtr<db::iDict> m_dbDict;
   std::unique_ptr<db::Char> m_char;
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("charDetail") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
