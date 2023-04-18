#ifndef ___cui_ani___
#define ___cui_ani___

#include "../cmn/win32.hpp"
#include "api.hpp"
#include "pen.hpp"
#include <functional>
#include <list>
#include <map>

namespace ani {

class delay {
public:
   delay() : nMSec(25), nSkip(0), m_nCnt(0) {}

   int nMSec;
   int nSkip;

   void sleep();

private:
   int m_nCnt;
};

class delayTweakKeystrokeMonitor : private cmn::iThread {
public:
   explicit delayTweakKeystrokeMonitor(delay& d)
   : m_d(d), m_wasTweaked(false), m_tc(*this) {}

   void add(char k, std::function<void(void)> f) { m_extra[k] = f; }

   void start() { m_tc.start(); }
   void stop();

   bool wasTweaked() const { return m_wasTweaked; }

private:
   virtual void run();

   delay& m_d;
   bool m_wasTweaked;
   cmn::osEvent m_stopSignal;
   cmn::threadController m_tc;
   std::map<char,std::function<void(void)> > m_extra;
};

// a bunch of drawings that happen at the same time
class frame {
public:
   void run(pen::object& pn);
   void add(std::function<void(pen::object&)> f) { m_sketches.push_back(f); }

private:
   std::list<std::function<void(pen::object&)> > m_sketches;
};

class iCanvas {
public:
   virtual frame& getFrame(size_t i) = 0;
};

class flipbook : public iCanvas {
public:
   explicit flipbook(delay& d) : m_d(d) {}

   void run(pen::object& pn);
   size_t size() const;
   virtual frame& getFrame(size_t i);

private:
   delay& m_d;
   std::map<size_t,frame> m_f;
};

class sequencer {
public:
   explicit sequencer(flipbook& f) : m_f(f) {}

   void simultaneous(const std::vector<std::function<void(iCanvas&)> >& a);

private:
   flipbook& m_f;
};

// "artists" are a pattern, not a type.  These offer up function usable with a sequencer
// to create effects

class prim {
public:
   static void lineLeftToRight(iCanvas& c, cui::pnt p, size_t l);
   static void lineRightToLeft(iCanvas& c, cui::pnt p, size_t l);
   static void box(frame& f, cui::pnt ul, size_t l, size_t h, pen::colors bgcol);
};

class attendance {
public:
   static void colorBox(iCanvas& c, size_t i);
   static void drawConnection(iCanvas& c, size_t i);
};

class outliner {
public:
   void outline(cui::pnt p, size_t l, size_t h, iCanvas& c);
   void restore(iCanvas& c);
};

} // namespace ani

#endif // ___cui_ani___
