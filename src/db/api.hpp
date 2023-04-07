#ifndef ___db_api___
#define ___db_api___

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

class staticDict {
public:
   const staticChar *findChar(size_t id);
   size_t numChars() const;
   const equip *findItem(size_t id);
};

class Char {
public:
   size_t id;
   size_t stars;
   size_t level;

   size_t weapon;
   size_t armor;
   size_t boots;
   size_t accessory;
};

} // namespace db

#endif // ___db_api___
