#ifndef ___cmn_string___
#define ___cmn_string___

#define WIN32_LEAN_AND_MEAN
#include <string>
#include <windows.h>

namespace cmn {

std::wstring widen(const std::string& nstr)
{
   int len = nstr.size();
   std::wstring wstr(len + 1, 0);
   mbstowcs(&wstr[0], nstr.c_str(), len);
   return wstr.c_str();
}

} // namespace cmn

#endif // ___cmn_string___
