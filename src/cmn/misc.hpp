#ifndef ___cmn_misc___
#define ___cmn_misc___

#include <stdexcept>

namespace cmn {

class runawayLoopCheck {
public:
   explicit runawayLoopCheck(size_t maxIterationsAllowed = 10000)
   : m_max(maxIterationsAllowed), m_i(0) {}

   void sanityCheck()
   {
      if(++m_i >= m_max)
         throw std::runtime_error("runaway loop detected!");
   }

private:
   const size_t m_max;
   size_t m_i;
};

} // namespace cmn

#endif // ___cmn_misc___
