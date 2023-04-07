#define WIN32_LEAN_AND_MEAN
#include "../tcatlib/api.hpp"
#include "api.hpp"
#include <windows.h>

namespace db {

static const staticChar gChars[] = {
   { kSsr, kEarth, "Corporate",  "Classics",     "Anime",      "Cannibal mickey mouse",      "spec" },
   { kSsr, kEarth, "Corporate",  "Classics",     "Anime",      "Drunken sailor Donald Duck", "spec" },
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
   { kSsr, kEarth, "Pets",       "None",         "None",       "Holly",                      "spec" },

   // 23 so far

   // bee swarm simulator
   // dagonrompa


#if 0
   { kSsr, kEarth, "caste", "sub", "hate", "name", "spec" },
#endif
};

} // namespace db

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
