#ifndef ___cui_pen___
#define ___cui_pen___

#include "api.hpp"
#include <iostream>

namespace pen {

class object {
public:
   explicit object(std::ostream& s) : m_s(s) {}

   std::ostream& str() { return m_s; }

private:
   std::ostream& m_s;
};

enum colors {
   kBlack,
   kRed,
   kGreen,
   kYellow,
   kBlue,
   kMagenta,
   kCyan,
   kWhite
};

class colorBase {
protected:
   colorBase(colors c, bool bright, const size_t table[][2])
   : m_c(table[c][bright ? 1 : 0]) {}

public:
   void insert(std::ostream& s) const { s << "\x1b[" << m_c << "m"; }

private:
   size_t m_c;
};

class fgcol : public colorBase {
public:
   explicit fgcol(colors c, bool bright = false);
};

class bgcol : public colorBase {
public:
   explicit bgcol(colors c, bool bright = false);
};

class moveTo {
public:
   explicit moveTo(const cui::pnt& p) : m_p(p) {}

public:
   void insert(std::ostream& s) const { s << "\x1b[" << m_p.y << ";" << m_p.x << "H"; }

private:
   cui::pnt m_p;
};

class clearScreen {};

} // namespace pen

inline std::ostream& operator<<(std::ostream& s, const pen::colorBase& v)
{ v.insert(s); return s; }

inline std::ostream& operator<<(std::ostream& s, const pen::moveTo& v)
{ v.insert(s); return s; }

inline std::ostream& operator<<(std::ostream& s, const pen::clearScreen&)
{ s << "\x1b[2J"; return s; }

#endif // ___cui_pen___
