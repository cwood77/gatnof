#include "../tcatlib/api.hpp"
#include "commandLineParser.hpp"

namespace console {

iCommandLineParser& commandLineParser::addVerb(iArgPattern& v)
{
   m_patterns.push_back(&v);
   return *this;
}

iCommand *commandLineParser::parse(int argc, const char *argv[])
{
   iArgPattern *pLast = NULL;
   for(int i=1;i<argc;i++)
   {
      bool matched = false;

      std::list<iArgPattern*>::iterator it = m_patterns.begin();
      for(;it!=m_patterns.end()&&!matched;++it)
      {
         if((*it)->tryMatchWord(argv[i]))
         {
            pLast = *it;
            pLast->adjustPatterns(m_patterns);
            matched = true;
         }
      }
   
      if(!matched)
         throw std::runtime_error("bad usage");
   }

   if(pLast)
      return pLast->complete();
   else
      return NULL;
}

tcatExposeTypeAs(commandLineParser,iCommandLineParser);

} // namespace console
