#ifndef ___cui_ani___
#define ___cui_ani___

#include <functional>
#include <list>
#include <map>

namespace cui { class control; }
namespace pen { class object; }

namespace ani {

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
   void run(pen::object& pn);
   virtual frame& getFrame(size_t i);

private:
   std::map<size_t,frame> m_f;
};

class sequencer {
public:
   explicit sequencer(flipbook& f) : m_f(f) {}

   void simultaneous(const std::vector<std::function<void(iCanvas&)> >& a);

private:
   flipbook& m_f;
};

//class canvasAdapter : public iCanvas {}

class outliner {
public:
   explicit outliner(cui::control& c) : m_c(c) {}

   void outline(iCanvas& c);

   void restore(iCanvas& c);

private:
   cui::control& m_c;
};

class blinker {
public:
   explicit blinker(cui::control& c) : m_c(c) {}

   void blink(iCanvas& c);

   void restore(iCanvas& c);

private:
   cui::control& m_c;
};

} // namespace ani

#endif // ___cui_ani___
