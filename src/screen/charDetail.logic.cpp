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

class equipInst {
public:
   equipInst() : eId(0), pE(NULL), charType(-1) {}

   size_t eId;
   const db::equip *pE;
   int charType;

   bool isEquipped() const { return charType == -2; }
   bool inUse() const { return charType >= 0; }
};

class equipSorter {
public:
   typedef equipInst *elt_t;
   bool operator()(const elt_t& pLhs, const elt_t& pRhs) const
   {
      // nothing first
      bool LisNothing = (pLhs->pE == NULL);
      bool RisNothing = (pRhs->pE == NULL);
      if(LisNothing != RisNothing) return LisNothing;
      // NB: guard pE against null by making sure _both_ aren't
      // nothing
      if(LisNothing) return false;

      // quality
      if(pLhs->pE->quality != pRhs->pE->quality)
         return pLhs->pE->quality > pRhs->pE->quality;

      // name
      auto nameCmp = ::strcmp(pLhs->pE->name,pRhs->pE->name);
      if(nameCmp != 0)
         return nameCmp < 0;

      // mine
      bool LisEquipped = pLhs->isEquipped();
      bool RisEquipped = pRhs->isEquipped();
      if(LisEquipped != RisEquipped) return LisEquipped;

      // in use
      bool LinUse = pLhs->inUse();
      bool RinUse = pRhs->inUse();
      if(LinUse != RinUse) return LinUse;

      // fallback
      return pLhs < pRhs;
   }
};

class equipList {
public:
   std::vector<equipInst>           vec;
   std::set<equipInst*,equipSorter> set;

   void clear() { set.clear(); vec.clear(); }
   void sort() { for(auto& x : vec) set.insert(&x); }
};

class equipListBuilder {
public:
   explicit equipListBuilder(equipList& l) : m_el(l) {}

   void rebuild(db::iDict& dbDict, sst::dict& acct, db::equipTypes filter, size_t myCharType)
   {
      m_el.clear();

      bool iAmEquipped = createLookupTable(acct,(size_t)filter,myCharType);
      createList(dbDict,acct,filter);
      createNothingValue(!iAmEquipped);

      m_el.sort();
   }

private:
   bool createLookupTable(sst::dict& acct, size_t equipSlot, size_t myCharType)
   {
      bool sawMyCharType = false;

      auto& Chars = acct["chars"].as<sst::dict>().asMap();
      for(auto it=Chars.begin();it!=Chars.end();++it)
      {
         auto& Char = it->second->as<sst::dict>();
         auto& equip = Char["equip"].as<sst::array>();
         auto val = equip[equipSlot].as<sst::mint>().get();
         if(val)
         {
            auto type = Char["type"].as<sst::mint>().get();
            const bool isMyCharType = (type == myCharType);
            m_table[val].insert(isMyCharType ? -2 : type);
            if(isMyCharType)
               sawMyCharType = true;
         }
      }

      return sawMyCharType;
   }

   void createList(db::iDict& dbDict, sst::dict& acct, db::equipTypes filter)
   {
      size_t first, count;
      dbDict.getItemRange(first,count);

      auto& inven = acct["items"].as<sst::dict>().asMap();
      for(auto it=inven.begin();it!=inven.end();++it)
      {
         auto type = it->second->as<sst::dict>()["type"].as<sst::mint>().get();
         if(! (first <= type && type < (first+count)) )
            continue; // not an item (e.g. maybe a shard?)

         auto& e = dbDict.findItem(type);
         if(e.type != filter)
            continue;

         auto cnt = it->second->as<sst::dict>()["amt"].as<sst::mint>().get();
         for(size_t i=0;i<cnt;i++)
            createItem(type,e);
      }
   }

   void createItem(size_t i, const db::equip& e)
   {
      // create item
      m_el.vec.push_back(equipInst());
      auto& entry = m_el.vec.back();
      entry.eId = i;
      entry.pE = &e;

      // associate with a char, if necessary
      auto& waiting = m_table[i];
      if(waiting.size())
      {
         entry.charType = *waiting.begin();
         waiting.erase(entry.charType);
      }
   }

   void createNothingValue(bool isEquipped)
   {
      m_el.vec.push_back(equipInst());
      auto& entry = m_el.vec.back();
      if(isEquipped)
         entry.charType = -2;
   }

   equipList& m_el;
   //       items => chars using them
   std::map<size_t,std::set<size_t> > m_table;
};

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

      // whole screen re-draw
      render();

      // setup int formatting
      setupControls();

      while(true)
      {
         // grab the char that's been selected by my parent
         reloadChar();

         // draw main panel form fields
         drawBasicCharFields();
         drawStats();
         drawCurrencies();

         // select / sort equip panel
         equipListBuilder(m_eList).rebuild(*m_dbDict,*m_acct,db::kBoots,m_char->getType());

         // draw equip panel
         m_subtableTitle.update("  BOOTS  ");
         drawEquip(pg);

         // handle user input
         cui::buttonHandler handler(m_error);
         handler.addCustom('0',[&](bool& stop){ equip( 0,stop); });
         handler.addCustom('1',[&](bool& stop){ equip( 1,stop); });
         handler.addCustom('2',[&](bool& stop){ equip( 2,stop); });
         handler.addCustom('3',[&](bool& stop){ equip( 3,stop); });
         handler.addCustom('4',[&](bool& stop){ equip( 4,stop); });
         handler.addCustom('5',[&](bool& stop){ equip( 5,stop); });
         handler.addCustom('6',[&](bool& stop){ equip( 6,stop); });
         handler.addCustom('7',[&](bool& stop){ equip( 7,stop); });
         handler.addCustom('8',[&](bool& stop){ equip( 8,stop); });
         handler.addCustom('9',[&](bool& stop){ equip( 9,stop); });
         handler.addCustom('A',[&](bool& stop){ equip(10,stop); });
         handler.addCustom('B',[&](bool& stop){ equip(11,stop); });
         handler.addCustom('C',[&](bool& stop){ equip(12,stop); });
         handler.add(m_backBtn,[&](bool& stop){ stop = true; });
         handler.add(m_levelUpBtn,[&](bool& stop) { boostChar("level-up",stop); });
         handler.add(m_starUpBtn,[&](bool& stop) { boostChar("star-up",stop); });
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

      const size_t N = m_table.size();
      for(size_t i=0;i<N;i++)
         rightJustifyMaxIntCtl(m_table[i].quality);
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

   void drawEquip(long pg)
   {
      const size_t N = m_table.size();
      size_t nSkip = N * pg;
      m_upBtn.dim("already at first page",nSkip==0);
      m_upBtn.redraw();

      m_eMap.clear();
      size_t i=0;
      for(auto *pElt : m_eList.set)
      {
         if(i < nSkip)
         {
            nSkip--;
            continue;
         }

         if(i == N)
            break;

         auto& row = m_table[i];
         row.eName.setFormatMode(pElt->isEquipped() ? 2 : 1);
         if(pElt->pE)
         {
            row.eRarity.update(db::fmtRaritiesFixedWidth(pElt->pE->rarity));
            row.quality.redraw(pElt->pE->quality);
            row.eName.redraw(pElt->pE->name);
            row.used.update(pElt->inUse() ? "X" : "");
         }
         else
         {
            row.eRarity.update("");
            row.quality.update(0);
            row.eName.redraw("<Nothing>");
            row.used.update("");
         }
         row.eName.setFormatMode(1);
         m_eMap[i] = pElt;

         i++;
      }

      m_downBtn.dim("already at last page",i!=N);
      m_downBtn.redraw();

      for(;i<N;i++)
      {
         auto& row = m_table[i];
         row.eRarity.update("");
         row.quality.erase();
         row.eName.redraw("");
         row.used.update("");
      }
   }

   void equip(size_t idx, bool& stop)
   {
      auto it = m_eMap.find(idx);
      if(it == m_eMap.end())
      {
         m_error.redraw("invalid equipment selection");
         return;
      }

      if(it->second->inUse())
      {
         m_error.redraw("equipment in use by another character");
         return;
      }

      std::stringstream sKey;
      sKey << m_char->getType();

      sst::dict req;
      req.add<sst::str>("char") = sKey.str();
      req.add<sst::str>("action") = "change-equip";
      req.add<sst::mint>("item") = it->second->eId;
      req.add<sst::mint>("item-type") = 2;

      auto& ch = m_svcMan->demand<net::iChannel>();
      ch.sendString("boostChar");
      ch.sendSst(req);

      auto err = ch.recvString();
      if(err.empty())
      {
         m_acct.reset(ch.recvSst());
         stop = true;
      }
      else
         m_error.redraw(err);
   }

   void boostChar(const std::string& mode, bool& stop)
   {
      std::stringstream sKey;
      sKey << m_char->getType();

      sst::dict req;
      req.add<sst::str>("char") = sKey.str();
      req.add<sst::str>("action") = mode;

      auto& ch = m_svcMan->demand<net::iChannel>();
      ch.sendString("boostChar");
      ch.sendSst(req);

      auto err = ch.recvString();
      if(err.empty())
      {
         m_acct.reset(ch.recvSst());
         stop = true;
      }
      else
         m_error.redraw(err);
   }

   tcat::typePtr<cmn::serviceManager> m_svcMan;
   std::unique_ptr<sst::dict>& m_acct;

   tcat::typePtr<db::iDict> m_dbDict;
   std::unique_ptr<db::Char> m_char;

   equipList m_eList;
   std::map<size_t,equipInst*> m_eMap;
};

class fac : public cui::plugInFactoryT<logic,cui::iLogic> {
public:
   fac() : cui::plugInFactoryT<logic,cui::iLogic>("charDetail") {}
};

tcatExposeTypeAs(fac,cui::iPlugInFactory);

} // anonymous namespace
