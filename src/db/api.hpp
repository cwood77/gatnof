#ifndef ___db_api___
#define ___db_api___

#include "../file/api.hpp"
#include <cstring>

namespace db {

enum rarities {
   kR,
   kSr,
   kSsr,
   kUr
};

// e -> w -> f -> ...
enum elements {
   kWater,
   kFire,
   kEarth
};

class equip {
public:
   rarities rarity;
   size_t quality;
   const char *name;
};

class staticChar {
public:
   rarities rarity;
   elements element;
   const char *caste;
   const char *subcaste;
   const char *hates;
   const char *name;
   const char *specialAttack;
};

class staticStat {
public:
   struct coef_t {
      size_t m;
      size_t b;
   } coef[4];

   size_t getBase(rarities r, size_t lvl) const;
};

class iDict {
public:
   virtual ~iDict() {}
   virtual const staticChar& findChar(size_t id) = 0;
   virtual size_t numChars() const = 0;
   virtual const equip& findItem(size_t id) = 0;
   virtual const staticStat& findStat() = 0;
};

class Char {
public:
   Char(iDict& d, sst::dict& overlay, int teamBonus);

   rarities rarity() { return m_pStatic->rarity; }
   elements element() { return m_pStatic->element; }
   std::string name() const { return m_pStatic->name; }

   //size_t getStars();
   //void setStars(size_t v);
   size_t getLevel();
   //void setLevel(size_t v);

   size_t atk(bool special) const { return getStat(special,getWeapon()); }
   size_t def() const             { return getStat(false,  getArmor()); }
   size_t agil() const            { return getStat(false,  getBoot()); }

   const equip *getEquip(size_t i) const { return m_equip[i]; }
   const equip *getWeapon() const    { return getEquip(0); }
   const equip *getArmor() const     { return getEquip(1); }
   const equip *getBoot() const      { return getEquip(2); }
   const equip *getAccessory() const { return getEquip(3); }

   size_t hp;
   int bonus;
   int userData;

private:
   void configureEquip(iDict& d);
   size_t getStat(bool special, const equip *pE) const;
   size_t applyBonus(size_t x) const;

   sst::dict& m_overlay;
   const staticChar *m_pStatic;
   const equip *m_equip[4];
   size_t m_baseStat;
};

class teamBonusCalculator {
public:
   void addChar(Char& c);
   void calculate(int& i);
};

class indivBonusCalculator {
public:
   indivBonusCalculator(sst::dict& environs) : m_environs(environs) {}

   void calculate(Char& p, Char& o);

private:
   void applyEnvirons(Char& c);
   void applyElemental(Char& c, Char& o);
   void finalize(Char& c);

   sst::dict& m_environs;
};

#include "api.ipp"

} // namespace db

#endif // ___db_api___
