#define WIN32_LEAN_AND_MEAN
#include "../console/log.hpp"
#include "../tcatlib/api.hpp"
#include "api.hpp"
#include "parse.hpp"
#include <sstream>
#include <stdexcept>
#include <windows.h>

namespace sst {

class serializer : public iSerializer {
public:
   virtual const char *write(node& n)
   {
      std::stringstream stream;

      write(n,stream);
      m_cache = stream.str();

      return m_cache.c_str();
   }

private:
   virtual void write(node& n, std::ostream& stream)
   {
      if(auto dn = dynamic_cast<str*>(&n))
      {
         stream << "\"" << dn->get() << "\"";
      }
      else if(auto dn = dynamic_cast<mint*>(&n))
      {
         stream << dn->get();
      }
      else if(auto dn = dynamic_cast<tf*>(&n))
      {
         stream << (dn->get() ? "true" : "false");
      }
      else if(auto dn = dynamic_cast<dict*>(&n))
      {
         stream << "{" << std::endl;
         adjustIndent(3);

         auto& values = dn->asMap();
         for(auto it=values.begin();it!=values.end();++it)
         {
            if(it!=values.begin())
               stream << "," << std::endl;

            stream << m_indent << "\"" << it->first << "\": ";
            write(*it->second,stream);
         }

         adjustIndent(-3);
         stream << std::endl << m_indent << "}";
      }
      else if(auto dn = dynamic_cast<array*>(&n))
      {
         stream << "[" << std::endl;
         adjustIndent(3);

         auto& values = dn->asVector();
         for(auto it=values.begin();it!=values.end();++it)
         {
            if(it!=values.begin())
               stream << "," << std::endl;

            stream << m_indent;
            write(**it,stream);
         }

         adjustIndent(-3);
         stream << std::endl << m_indent << "]";
      }
      else
         throw std::runtime_error("unknown note type in serializer");
   }

   void adjustIndent(int offset)
   {
      size_t newIndent = m_indent.length() + offset;
      m_indent = std::string(newIndent,' ');
   }

   std::string m_cache;
   std::string m_indent;
};

tcatExposeTypeAs(serializer,iSerializer);

class deserializer : public iDeserializer {
public:
   virtual dict *parse(const char *pPtr, const iNodeFactory& f)
   {
      lexor l(pPtr);
      parser p(l,f);
      return (dict*)p.parseConfig();
   }
};

tcatExposeTypeAs(deserializer,iDeserializer);

} // namespace sst

tcatImplServer();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
