#define WIN32_LEAN_AND_MEAN
#include "../tcatlib/api.hpp"
#include "api.hpp"
#include <conio.h>
#include <stdexcept>
#include <windows.h>

namespace cui { 

class factory : public iFactory {
public:
   virtual iObject& _create(const std::string& typeId, const std::string& id)
   {
      tcat::typeSet<iPlugInFactory> parts;
      auto n = parts.size();
      for(size_t i=0;i<n;i++)
      {
         auto *pObj = parts[i]->tryCreate(typeId,id);
         if(pObj)
            return *pObj;
      }
      throw std::runtime_error("no iPlugInFactory satisified request");
   }
};

tcatExposeTypeAs(factory,iFactory);

class userInput : public iUserInput {
public:
   virtual char getKey() { while(::kbhit()) { ::getch(); } return ::getch(); }
};

tcatExposeTypeAs(userInput,iUserInput);

} // namespace cui

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
