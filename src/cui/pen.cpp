#include "pen.hpp"

namespace pen {

static const size_t gFgColors[][2] = {
   /* kBlack   */ { 30, 90 },
   /* kRed     */ { 31, 91 },
   /* kGreen   */ { 32, 92 },
   /* kYellow  */ { 33, 93 },
   /* kBlue    */ { 34, 94 },
   /* kMagenta */ { 35, 95 },
   /* kCyan    */ { 36, 96 },
   /* kWhite   */ { 37, 97 }
};

static const size_t gBgColors[][2] = {
   /* kBlack   */ { 40, 100 },
   /* kRed     */ { 41, 101 },
   /* kGreen   */ { 42, 102 },
   /* kYellow  */ { 43, 103 },
   /* kBlue    */ { 44, 104 },
   /* kMagenta */ { 45, 105 },
   /* kCyan    */ { 46, 106 },
   /* kWhite   */ { 47, 107 }
};

fgcol::fgcol(colors c, bool bright) : colorBase(c,bright,gFgColors) {}

bgcol::bgcol(colors c, bool bright) : colorBase(c,bright,gBgColors) {}

} // namespace pen