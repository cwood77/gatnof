#define WIN32_LEAN_AND_MEAN
#include "../cmn/win32.hpp"
#include "../file/api.hpp"
#include "../tcatlib/api.hpp"
#include "log.hpp"
#include <cstdio>
#include <stdarg.h>
#include <windows.h>

namespace console {

class stdLog : public iLog {
public:
   enum level {
      kInfo    = 0x1,
      kVerbose = 0x2,
      kDebug   = 0x4,
      kTemp    = 0x8
   };

   stdLog()
   : m_pSink(NULL), m_indent(0), m_freshLine(false)
   , m_autoFlush(true), m_prefix(true), m_passFilter(0xFFFF) {}

   virtual void release() { delete this; }

   virtual void configure(sst::dict& d)
   {
      m_autoFlush  = d.getOpt<sst::tf>("log:flush",true);
      m_prefix     = d.getOpt<sst::tf>("log:prefix",true);
      m_passFilter = d.getOpt<sst::mint>("log:passFilter",0x1);
   }

   virtual void configureForBackground()
   {
      m_autoFlush  = true;
      m_prefix     = true;
      m_passFilter = 0xFFFF;
   }

   virtual void sink(iLogSink *pSink)
   {
      m_pSink = pSink;
   }

   virtual void adjustIndent(int i)
   {
      m_indent += i;
   }

   virtual void writeLnInfo(const std::string& fmt, ...)
   {
      char buffer[2000];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buffer,2000,fmt.c_str(),args);
      va_end(args);
      write(kInfo,buffer,true);
   }

   virtual void writeLnVerbose(const std::string& fmt, ...)
   {
      char buffer[2000];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buffer,2000,fmt.c_str(),args);
      va_end(args);
      write(kVerbose,buffer,true);
   }

   virtual void writeLnDebug(const std::string& fmt, ...)
   {
      char buffer[2000];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buffer,2000,fmt.c_str(),args);
      va_end(args);
      write(kDebug,buffer,true);
   }

   virtual void writeLnTemp(const std::string& fmt, ...)
   {
      char buffer[2000];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buffer,2000,fmt.c_str(),args);
      va_end(args);
      write(kTemp,buffer,true);
   }

private:
   void write(level l, const std::string& text, bool line)
   {
      cmn::autoLock _l(m_m);

      std::string fulltext;

      if(m_freshLine)
         fulltext += std::string(m_indent,' ');

      fulltext += text;

      if(line)
      {
         fulltext += "\r\n";
         m_freshLine = true;
      }
      else
         m_freshLine = false;

      writeInternal(l,fulltext);
   }

   void writeInternal(level l, const std::string& text)
   {
      if((l & m_passFilter) == 0)
         return; // filtered out

      std::string toSend = text;
      if(m_prefix)
      {
         if(l == kInfo)
            toSend = "";
         else if(l == kVerbose)
            toSend = "[verb] ";
         else if(l == kDebug)
            toSend = "[dbg] ";
         else if(l == kTemp)
            toSend = "[tmp] ";
         toSend += text;
      }

      if(m_pSink)
      {
         m_pSink->writeWords(toSend);
         if(m_autoFlush)
            m_pSink->flush();
      }
   }

   cmn::mutex m_m;

   iLogSink *m_pSink;
   size_t m_indent;
   bool m_freshLine;

   bool m_autoFlush;
   bool m_prefix;
   size_t m_passFilter;
};

tcatExposeTypeAs(stdLog,iLog);

} // namespace console

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
