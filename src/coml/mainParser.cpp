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

void mainParser::loadAndVerify(const std::string& path, std::vector<std::string>& lines, int& height)
{
   // load all the lines
   {
      std::ifstream in(path.c_str());
      while(in.good())
      {
         std::string line;
         std::getline(in,line);
         lines.push_back(line);
      }
   }

   // enforce ID
   if(::strncmp(lines[0].c_str(),"coml v1 ",8) != 0)
      throw std::runtime_error("bad coml header");

   // read index
   height = 0;
   ::sscanf(lines[0].c_str(),"coml v1 %d",&height);
}

void mainParser::parseObjectLine(const std::string& line, bool& stop, objectTable& oTable)
{
   if(stop || line.empty())
      return;
   stop = (line == "; comment");
   if(stop)
      return;

   int number = 0;
   ::sscanf(line.c_str(),"%d=",&number);
   const char *pThumb = line.c_str();
   eatUntil(pThumb,'=');

   auto& list = oTable.create(number);
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
      char buffer1[1024];
      char buffer2[1024];
      int n = 0;
      ::sscanf(pThumb,"str:%[^/]/%d/%d/%[^/]/%n",
         buffer1,&(pObj->length),&(pObj->height),buffer2,&n);
      pObj->name = buffer1;
      pObj->format1 = buffer2;
      pObj->format2 = pThumb + n;
      pObj->baseType = "cui::stringControl";
   }
   else if(::strncmp(pThumb,"int:",4)==0)
   {
      auto *pObj = new controlObject();
      list.push_back(pObj);
      char buffer1[1024];
      char buffer2[1024];
      int n = 0;
      ::sscanf(pThumb,"int:%[^/]/%d/%d/%[^/]/%n",
         buffer1,&(pObj->length),&(pObj->height),buffer2,&n);
      pObj->name = buffer1;
      pObj->format1 = buffer2;
      pObj->format2 = pThumb + n;
      pObj->baseType = "cui::intControl";
   }
   else if(::strncmp(pThumb,"btn:",4)==0)
   {
      auto *pObj = new buttonControlObject();
      list.push_back(pObj);
      char buffer1[1024];
      char buffer2[1024];
      char buffer3[1024];
      char buffer4[1024];
      char buffer5[1024];
      int n = 0;
      ::sscanf(pThumb,"btn:%[^/]/%d/%d/%[^/]/%[^/]/%[^/]/%[^/]/%n",
         buffer1,&(pObj->length),&(pObj->height),buffer2,buffer3,buffer4,buffer5,&n);
      pObj->name = buffer1;
      pObj->face = buffer2;
      pObj->format1 = buffer3;
      pObj->format2 = buffer4;
      pObj->format3 = buffer5;
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

} // namespace coml
