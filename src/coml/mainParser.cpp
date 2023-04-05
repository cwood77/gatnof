#include "mainParser.hpp"
#include <fstream>
#include <cstring>
#include <sstream>

namespace coml {

void parserBase::eatUntil(const char*& pThumb, char c)
{
   for(;*pThumb!=c;++pThumb);
   ++pThumb;
}

void mainParser::loadLines(const std::string& inputPath)
{
   std::ifstream in(inputPath.c_str());
   while(in.good())
   {
      std::string line;
      std::getline(in,line);
      m_ir.lines.push_back(line);
   }
}

void mainParser::readHeader()
{
   // enforce ID
   if(::strncmp(m_ir.lines[0].c_str(),"coml v1 ",8) != 0)
      throw std::runtime_error("bad coml header");

   // read index
   ::sscanf(m_ir.lines[0].c_str(),"coml v1 %d",&m_ir.height);
}

void mainParser::buildObjectTable()
{
   for(size_t i=1+m_ir.height;;i++)
   {
      if(i >= m_ir.lines.size())
         break; // done

      bool stop = false;
      parseObjectLine(m_ir.lines[i],stop);

      if(stop)
         break; // rest are comments
   }
}

void mainParser::parseObjectLine(const std::string& line, bool& stop)
{
   if(line.empty())
      return;

   stop = (line == "; begin comments");
   if(stop)
      return;

   if(line.c_str()[0] == ';')
      return;

   int number = 0;
   ::sscanf(line.c_str(),"%d=",&number);
   const char *pThumb = line.c_str();
   eatUntil(pThumb,'=');

   auto& list = m_ir.oTable.create(number);
   parseObject(pThumb,list);
}

void mainParser::parseObject(const char*& pThumb, std::list<iObject*>& list)
{
   if(::strncmp(pThumb,"txt:",4)==0)
   {
      auto *pObj = new textObject();
      list.push_back(pObj);
      pObj->payload = pThumb + 4;
   }
   else if(::strncmp(pThumb,"str:",4)==0)
   {
      auto *pObj = new controlObject();
      list.push_back(pObj);
      pObj->baseType = "cui::stringControl";
      parseControlObject(pThumb+4,*pObj);
   }
   else if(::strncmp(pThumb,"int:",4)==0)
   {
      auto *pObj = new controlObject();
      list.push_back(pObj);
      pObj->baseType = "cui::intControl";
      parseControlObject(pThumb+4,*pObj);
   }
   else if(::strncmp(pThumb,"btn:",4)==0)
   {
      auto *pObj = new buttonControlObject();
      list.push_back(pObj);
      char name[1024];
      char face[1024];
      char fmt1[1024];
      char fmt2[1024];
      char fmt3[1024];
      int n = 0;
      ::sscanf(pThumb,"btn:%[^/]/%d/%d/%[^/]/%[^/]/%[^/]/%[^/]/%n",
         name,&(pObj->length),&(pObj->height),face,fmt1,fmt2,fmt3,&n);
      pObj->name = name;
      pObj->face = face;
      pObj->format1 = fmt1;
      pObj->format2 = fmt2;
      pObj->format3 = fmt3;
      pObj->format4 = pThumb + n;
      pObj->baseType = "cui::buttonControl";
   }
   else
   {
      std::stringstream stream;
      stream << "unknown object '" << pThumb << "'";
      throw std::runtime_error(stream.str());
   }
}

void mainParser::parseControlObject(const char *pThumb, controlObject& o)
{
   char name[1024];
   char fmt1[1024];
   int n = 0;
   ::sscanf(pThumb,"%[^/]/%d/%d/%[^/]/%n",
      name,&(o.length),&(o.height),fmt1,&n);
   o.name = name;
   o.format1 = fmt1;
   o.format2 = pThumb + n;
}

} // namespace coml
