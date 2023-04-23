#define WIN32_LEAN_AND_MEAN
#include "../tcatlib/api.hpp"
#include "api.hpp"
#include <windows.h>

namespace db {

static const size_t kReservedCharRange = 9000;

static const equip gEquip[] = {
   { kR,   kWeapon,     1, "Folding chair"              },
   { kR,   kWeapon,     5, "Hugs"                       },
   { kSr,  kWeapon,    10, "Nail clippers"              },
   { kSsr, kWeapon,    15, "Rusty butterknife"          },
   { kUr,  kWeapon,    20, "Dirty underwear on a stick" },
   { kR,   kArmor,      1, "Revealing swimsuit"         },
   { kSsr, kArmor,     10, "Saran wrap"                 },
   { kUr,  kArmor,     20, "Tin foil hat"               },
   { kR,   kBoots,      1, "Penny loafers"              },
   { kSsr, kBoots,     10, "Scuba flippers"             },
   { kUr,  kBoots,     20, "Pleather chaps"             },
   //{ kSsr, kAccessory, 20, ""          },
};

static const staticChar gChars[] = {
   { kR,   kEarth, "Corporate",  "Classics",     "Anime",      "Cannibal mickey mouse",      "spec" },
   { kSr,  kEarth, "Corporate",  "Classics",     "Anime",      "Drunken sailor Donald Duck", "spec" },
   { kSsr, kEarth, "Corporate",  "Classics",     "Anime",      "Vampire Tinkerbell",         "spec" },
   { kSsr, kEarth, "Corporate",  "Princesses",   "Anime",      "Necromancer Elsa",           "spec" },
   { kSsr, kEarth, "Corporate",  "Princesses",   "Anime",      "Seawitch Ariel",             "spec" },
   { kSsr, kEarth, "Corporate",  "Princesses",   "Anime",      "Briar Rose",                 "spec" },
   { kSsr, kEarth, "Anime",      "Iruma-kun",    "Presidents", "Iruma Suzuki",               "spec" },
   { kSsr, kEarth, "Anime",      "Iruma-kun",    "Presidents", "Alice Asmodeus",             "spec" },
   { kSsr, kEarth, "Anime",      "Iruma-kun",    "Presidents", "Clara Valac",                "spec" },
   { kSsr, kEarth, "Anime",      "Iruma-kun",    "Presidents", "Ameri Azazel",               "spec" },
   { kSsr, kEarth, "Anime",      "Iruma-kun",    "Presidents", "Grandpa Sullivan",           "spec" },
   { kSsr, kEarth, "Anime",      "Iruma-kun",    "Presidents", "Bulter Opera",               "spec" },
   { kSsr, kEarth, "Anime",      "Demon Slayer", "Corporate",  "Demon sister Nezuko-chan",   "spec" },
   { kSsr, kEarth, "Anime",      "Demon Slayer", "Corporate",  "Boar-head guy",              "spec" },
   { kSsr, kEarth, "Anime",      "Demon Slayer", "Corporate",  "Good brother Tanjiro",       "spec" },
   { kSsr, kEarth, "Anime",      "Demon Slayer", "Corporate",  "Butterfly",                  "spec" },
   { kSsr, kEarth, "Anime",      "Demon Slayer", "Corporate",  "Michael Jackson",            "spec" },
   { kSsr, kEarth, "Presidents", "Princesses",   "Everyone",   "Donald J Trump",             "Fake news" },
   { kSsr, kEarth, "Presidents", "Politicians",  "Anime",      "Barack Obama",               "Toothy grin" },
   { kSsr, kEarth, "Presidents", "Politicians",  "Anime",      "Kamela Harris",              "spec" },
   { kSsr, kEarth, "Presidents", "Politicians",  "Anime",      "President Biden",            "Where am I?" },
   { kSsr, kEarth, "Pets",       "All",          "None",       "Austin the Doge",            "spec" },
   { kUr,  kEarth, "Pets",       "None",         "None",       "Holly",                      "spec" },

   // 23 so far

   // bee swarm simulator
   // dagonrompa

#if 0
   { kSsr, kEarth, "caste", "sub", "hate", "name", "spec" },
#endif
};

static const staticStat gStats = {
   {
      //          m   b      1-60 range
      /*   R */ { 1,  0 }, //   1-60
      /*  SR */ { 1, 40 }, // 41-100
      /* SSR */ { 2,  0 }, //  2-120
      /*  UR */ { 2, 40 }  // 82-200
   }
};

#if 0
static const staticStat gStats[] = {
   // lvl      atk  def  agl
   /*   1 */ {   1,   8,   8 }, // 1 star
   /*   2 */ {  12,  11,  11 },
   /*   3 */ {  13,  17,  17 },
   /*   4 */ {  24,  20,  20 },
   /*   5 */ {  25,  25,  25 },
   /*   6 */ {  36,  30,  30 },
   /*   7 */ {  37,  35,  35 },
   /*   8 */ {  48,  40,  40 },
   /*   9 */ {  49,  45,  45 },
   /*  10 */ {  50,  50,  50 },
   /*  11 */ {  51,  51,  51 }, // 2 star
   /*  12 */ {  52,  52,  52 },
   /*  13 */ {  53,  53,  53 },
   /*  14 */ {  54,  54,  54 },
   /*  15 */ {  55,  55,  55 },
   /*  16 */ {  56,  56,  56 },
   /*  17 */ {  57,  57,  57 },
   /*  18 */ {  58,  58,  58 },
   /*  19 */ {  59,  59,  59 },
   /*  20 */ {  60,  60,  60 },
   /*  21 */ {  61,  61,  61 }, // 3 star
   /*  22 */ {  62,  62,  62 },
   /*  23 */ {  63,  63,  63 },
   /*  24 */ {  64,  64,  64 },
   /*  25 */ {  65,  65,  65 },
   /*  26 */ {  66,  66,  66 },
   /*  27 */ {  67,  67,  67 },
   /*  28 */ {  68,  68,  68 },
   /*  29 */ {  69,  69,  69 },
   /*  30 */ {  70,  70,  70 },
   /*  31 */ {  71,  71,  71 }, // 4 star
   /*  32 */ {  72,  72,  72 },
   /*  33 */ {  73,  73,  73 },
   /*  34 */ {  74,  74,  74 },
   /*  35 */ {  75,  75,  75 },
   /*  36 */ {  76,  76,  76 },
   /*  37 */ {  77,  77,  77 },
   /*  38 */ {  78,  78,  78 },
   /*  39 */ {  79,  79,  79 },
   /*  40 */ {  80,  80,  80 },
   /*  41 */ {  81,  81,  81 }, // 5 star
   /*  42 */ {  82,  82,  82 },
   /*  43 */ {  83,  83,  83 },
   /*  44 */ {  84,  84,  84 },
   /*  45 */ {  85,  85,  85 },
   /*  46 */ {  86,  86,  86 },
   /*  47 */ {  87,  87,  87 },
   /*  48 */ {  88,  88,  88 },
   /*  49 */ {  89,  89,  89 },
   /*  50 */ {  90,  90,  90 },
   /*  51 */ {  91,  91,  91 }, // 6 star
   /*  52 */ {  92,  92,  92 },
   /*  53 */ {  93,  93,  93 },
   /*  54 */ {  94,  94,  94 },
   /*  55 */ {  95,  95,  95 },
   /*  56 */ {  96,  96,  96 },
   /*  57 */ {  97,  97,  97 },
   /*  58 */ {  98,  98,  98 },
   /*  59 */ {  99,  99,  99 },
   /*  60 */ { 100, 100, 100 }
};
#endif

class staticDict : public iDict {
public:
   virtual const staticChar& findChar(size_t id);
   virtual size_t numChars() const;
   virtual const equip& findItem(size_t id);
   virtual void getItemRange(size_t& first, size_t& count);
   virtual const staticStat& findStat() { return gStats; }
};

const staticChar& staticDict::findChar(size_t id)
{
   const size_t nCnt = numChars();
   if(id >= nCnt)
      throw std::runtime_error("char range out of bounds");
   return gChars[id];
}

size_t staticDict::numChars() const
{
   static const size_t nCnt = sizeof(gChars) / sizeof(staticChar);
   return nCnt;
}

const equip& staticDict::findItem(size_t id)
{
   if(id < kReservedCharRange)
      throw std::runtime_error("char id passed to findItem");

   id -= kReservedCharRange; // rebase
   static const size_t nCnt = sizeof(gEquip) / sizeof(equip);
   if(id >= nCnt)
      throw std::runtime_error("equip range out of bounds");
   return gEquip[id];
}

void staticDict::getItemRange(size_t& first, size_t& count)
{
   static const size_t nCnt = sizeof(gEquip) / sizeof(equip);
   first = kReservedCharRange;
   count = nCnt;
}

tcatExposeTypeAs(staticDict,iDict);

} // namespace db

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
