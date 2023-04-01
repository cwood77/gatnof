#define WIN32_LEAN_AND_MEAN
#include "../cmn/service.hpp"
#include "../tcatlib/api.hpp"
#include <windows.h>

namespace cmn {

tcatExposeSingletonTypeAs(serviceManager,serviceManager);

} // namespace cmn

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
