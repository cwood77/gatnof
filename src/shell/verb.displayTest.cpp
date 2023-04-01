#define WIN32_LEAN_AND_MEAN
#include "../console/arg.hpp"
#include "../console/log.hpp"
#include "../cui/pen.hpp"
#include "../tcatlib/api.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <wchar.h>
#include <windows.h>

namespace {

class displayTestCommand : public console::iCommand {
public:
   virtual void run(console::iLog& l);
};

class myVerb : public console::globalVerb {
protected:
   virtual console::verbBase *inflate()
   {
      std::unique_ptr<console::verbBase> v(
         new console::verb<displayTestCommand>("--displayTest"));

      return v.release();
   }
} gVerb;

void displayTestCommand::run(console::iLog& l)
{
   // example code from MicroSoft

    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return;// GetLastError();
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        return;// GetLastError();
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        return;// GetLastError();
    }

    // Try some Set Graphics Rendition (SGR) terminal escape sequences
    printf("\x1b[31mThis text has a red foreground using SGR.31.\r\n");
    wprintf(L"\x1b[1mThis text has a bright (bold) red foreground using SGR.1 to affect the previous color setting.\r\n");
    wprintf(L"\x1b[mThis text has returned to default colors using SGR.0 implicitly.\r\n");
    wprintf(L"\x1b[34;46mThis text shows the foreground and background change at the same time.\r\n");
    wprintf(L"\x1b[0mThis text has returned to default colors using SGR.0 explicitly.\r\n");
    wprintf(L"\x1b[31;32;33;34;35;36;101;102;103;104;105;106;107mThis text attempts to apply many colors in the same command. Note the colors are applied from left to right so only the right-most option of foreground cyan (SGR.36) and background bright white (SGR.107) is effective.\r\n");
    wprintf(L"\x1b[39mThis text has restored the foreground color only.\r\n");
    wprintf(L"\x1b[49mThis text has restored the background color only.\r\n");
    wprintf(L"\x1b[s\x1b[1A\x1b[5CTEXT\x1b[u");
    wprintf(L"chris was here\r\n");

    // https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences

    {
       pen::object p(std::cout);
       p.str() << pen::clearScreen();
       p.str() << pen::fgcol(pen::kYellow) << "hello " << pen::bgcol(pen::kBlue) << "world" << std::endl;
    }

    for(size_t i=0;i<11;i++)
       for(size_t j=0;j<10;j++)
          std::cout << j;
    std::cout << std::endl;
    for(size_t i=0;i<11;i++)
    {
       std::cout << i;
       for(size_t j=0;j<9;j++)
          std::cout << " ";
    }
}

} // anonymous namespace
