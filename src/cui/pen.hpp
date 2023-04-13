#ifndef ___cui_pen___
#define ___cui_pen___

#include "api.hpp"
#include <iostream>

namespace pen {

class object {
public:
   static void setupStdOut();

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
   kWhite,
   kDefault
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

   void insert(std::ostream& s) const { s << "\x1b[" << m_p.y << ";" << m_p.x << "H"; }

private:
   cui::pnt m_p;
};

class clearScreen {};

class showCursor {
public:
   explicit showCursor(bool mode = true) : mode(mode) {}

   bool mode;
};

class block {
public:
   explicit block(size_t o, size_t n = 1) : opacity(o), n(n) {}

   void insert(std::ostream& s) const;

   size_t opacity;
   size_t n;
};

} // namespace pen

inline std::ostream& operator<<(std::ostream& s, const pen::colorBase& v)
{ v.insert(s); return s; }

inline std::ostream& operator<<(std::ostream& s, const pen::moveTo& v)
{ v.insert(s); return s; }

inline std::ostream& operator<<(std::ostream& s, const pen::clearScreen&)
{ s << "\x1b[2J"; return s; }

inline std::ostream& operator<<(std::ostream& s, const pen::showCursor& v)
{ s << "\x1b[?" << (v.mode ? "25h" : "25l"); return s; }

inline std::ostream& operator<<(std::ostream& s, const pen::block& v)
{ v.insert(s); return s; }

#endif // ___cui_pen___
