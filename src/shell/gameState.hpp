#ifndef ___shell_gameState___
#define ___shell_gameState___

namespace shell {

class gameState {
public:
   gameState(const std::string& a, const std::string& ip)
   : accountName(a), serverIp(ip) {}

   const std::string accountName;
   const std::string serverIp;
};

} // namespace shell

#endif // ___shell_gameState___
