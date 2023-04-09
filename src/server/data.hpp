#ifndef ___server_data___
#define ___server_data___

#include "../cmn/win32.hpp"
#include "../file/api.hpp"

namespace server {

extern cmn::mutex *gDataLock;
extern sst::dict *gServerData;

} // namespace server

#endif // ___server_data___
