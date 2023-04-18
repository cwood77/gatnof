#ifndef ___shell_gameState___
#define ___shell_gameState___

namespace shell {

class gameState {
public:
   gameState(const std::string& a, const std::string& ip)
   : accountName(a), serverIp(ip), nMSec(0), nSkip(0), doBattleAni(true) {}

   const std::string accountName;
   const std::string serverIp;

   int nMSec;
   int nSkip;
   bool doBattleAni;
};

} // namespace shell

#endif // ___shell_gameState___
