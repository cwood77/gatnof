#define WIN32_LEAN_AND_MEAN
#include "pen.hpp"
#include <stdexcept>
#include <windows.h>

namespace pen {

static const size_t gFgColors[][2] = {
   /* kBlack   */ { 30, 90 },
   /* kRed     */ { 31, 91 },
   /* kGreen   */ { 32, 92 },
   /* kYellow  */ { 33, 93 },
   /* kBlue    */ { 34, 94 },
   /* kMagenta */ { 35, 95 },
   /* kCyan    */ { 36, 96 },
   /* kWhite   */ { 37, 97 },
   /* kDefault */ { 39, 39 }
};

static const size_t gBgColors[][2] = {
   /* kBlack   */ { 40, 100 },
   /* kRed     */ { 41, 101 },
   /* kGreen   */ { 42, 102 },
   /* kYellow  */ { 43, 103 },
   /* kBlue    */ { 44, 104 },
   /* kMagenta */ { 45, 105 },
   /* kCyan    */ { 46, 106 },
   /* kWhite   */ { 47, 107 },
   /* kDefault */ { 49,  49 }
};

void object::setupStdOut()
{
   // Set output mode to handle virtual terminal sequences

   HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
   if (hOut == INVALID_HANDLE_VALUE)
      throw std::runtime_error("pen setup: can't acquire stdout handle");

   DWORD dwMode = 0;
   if (!GetConsoleMode(hOut, &dwMode))
      throw std::runtime_error("pen setup: can't query console mode");

   dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
   if (!SetConsoleMode(hOut, dwMode))
      throw std::runtime_error("pen setup: can't enabe virtual terminal processing");
}

fgcol::fgcol(colors c, bool bright) : colorBase(c,bright,gFgColors) {}

bgcol::bgcol(colors c, bool bright) : colorBase(c,bright,gBgColors) {}

void block::insert(std::ostream& s) const
{
   for(size_t i=0;i<n;i++)
      if(opacity == 0)
         s << "\xdb";
      else if(opacity == 1)
         s << "\xb2";
      else if(opacity == 2)
         s << "\xb1";
      else if(opacity == 3)
         s << "\xb0";
}

} // namespace pen
