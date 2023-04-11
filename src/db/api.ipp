inline const char *fmtRaritiesFixedWidth(rarities r)
{
   static const char *strs[] = { "  R", " SR", "SSR", " UR" };
   return strs[r];
}

inline const char *fmtElementsFixedWidth(elements e)
{
   static const char *strs[] = { "water", "fire  ", "earth" };
   return strs[e];
}

inline size_t staticStat::getBase(rarities r, size_t lvl) const
{
   return (coef[r].m * lvl) + coef[r].b;
}

inline Char::Char(iDict& d, sst::dict& overlay, int teamBonus)
: hp(100)
, bonus(teamBonus)
, userData(0)
, m_overlay(overlay)
, m_pStatic(NULL)
, m_baseStat(0)
{
   m_pStatic = &d.findChar(overlay["type"].as<sst::mint>().get());

   configureEquip(d);

   m_baseStat = d.findStat().getBase(rarity(),getLevel());
}

inline size_t Char::getType()
{
   return m_overlay["type"].as<sst::mint>().get();
}

inline size_t Char::getStars()
{
   return m_overlay["stars"].as<sst::mint>().get();
}

inline size_t Char::getLevel()
{
   return m_overlay["level"].as<sst::mint>().get();
}

inline void Char::configureEquip(iDict& d)
{
   ::memset(&m_equip,0,sizeof(m_equip));

   auto& e = m_overlay["equip"].as<sst::array>();

   auto id = e[0].as<sst::mint>().get();
   if(id)
      m_equip[0] = &d.findItem(id);

   id = e[1].as<sst::mint>().get();
   if(id)
      m_equip[1] = &d.findItem(id);

   id = e[2].as<sst::mint>().get();
   if(id)
      m_equip[2] = &d.findItem(id);

   id = e[3].as<sst::mint>().get();
   if(id)
      m_equip[3] = &d.findItem(id);
}

inline size_t Char::getStat(bool special, const equip *pE) const
{
   size_t x = m_baseStat;

   if(pE)
      x += pE->quality;

   if(special)
      x += (::rand() % 20);

   return applyBonus(x);
}

inline size_t Char::applyBonus(size_t x) const
{
   if(bonus == 2)
   {
      return x*2;
   }
   else if(bonus == 1)
   {
      return x*1.5;
   }
   else if(bonus == 0)
   {
      return x;
   }
   else if(bonus == -1)
   {
      return (x*0.5)+1;
   }
   else
      throw std::runtime_error("unsupported bonus ISE");
}

inline std::string fmtStarsFixedWidth(size_t n)
{
   return std::string(6-n,' ') + std::string(n,'*');
}

inline int teamBonusCalculator::calculate()
{
   if(m_chars.size() != 5)
      return 0;

   std::string subcaste;
   for(auto *pCh : m_chars)
   {
      if(std::string("All") == pCh->subcaste) continue;

      if(subcaste.empty())
         subcaste = pCh->subcaste;
      else
      {
         if(subcaste != pCh->subcaste)
            return 0;
      }
   }
   return 2;
}

inline void indivBonusCalculator::calculate(Char& p, Char& o)
{
   applyEnvirons(p);
   applyEnvirons(o);

   applyElemental(p,o);
   applyElemental(o,p);

   finalize(p);
   finalize(o);
}

inline void indivBonusCalculator::applyEnvirons(Char& c) // TODO unimpled
{
}

inline void indivBonusCalculator::applyElemental(Char& c, Char& o)
{
   bool advantage = false;

   switch(c.element())
   {
      case kWater:
         advantage = (o.element() == kFire);
         break;
      case kFire:
         advantage = (o.element() == kEarth);
         break;
      case kEarth:
         advantage = (o.element() == kWater);
         break;
   }

   if(advantage)
      c.bonus++;
}

inline void indivBonusCalculator::finalize(Char& c)
{
   if(c.bonus < -1)
      c.bonus = -1;
   else if(c.bonus > 2)
      c.bonus = 2;
}
