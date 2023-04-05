#include "msg.hpp"

namespace server {

msgRegistry& msgRegistry::get()
{
   static msgRegistry self;
   return self;
}

} // namespace server
