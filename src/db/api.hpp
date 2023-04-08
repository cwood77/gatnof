#ifndef ___db_api___
#define ___db_api___

#include "../file/api.hpp"

// "items": [
//    {
//       "amt": 20,
//       "type": 0
//    },
//    {
//       "amt": 1,
//       "type": 1
//    }
// ],
// "chars": {
//    "7": {
//       "equip": [
//          1
//       ],
//       "level": 3,
//       "stars": 1,
//       "type": 7
//    }
// },

namespace db {

enum rarities {
   kUr,
   kSsr,
   kSr,
   kR
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

class iDict {
public:
   virtual ~iDict() {}
   virtual const staticChar& findChar(size_t id) = 0;
   virtual size_t numChars() const = 0;
   virtual const equip& findItem(size_t id) = 0;
};

class Char {
public:
   Char(iDict& d, sst::dict& overlay);

   std::string name() const { return m_pStatic->name; }
   size_t hp;

private:
   sst::dict& m_overlay;
   const staticChar *m_pStatic;
};

#include "api.ipp"

} // namespace db

#endif // ___db_api___
