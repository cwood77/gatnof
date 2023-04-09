#include "ir.hpp"
#include <cstring>
#include <stdexcept>

namespace coml {

void ir::computeName(const std::string& comlPath)
{
   const char *pThumb = ::strstr(comlPath.c_str(),"/screen.") + 8;
   if(!pThumb)
      throw std::runtime_error("bad coml file path");
   name = std::string(pThumb,::strlen(pThumb) - 5);
}

} // namespace coml
