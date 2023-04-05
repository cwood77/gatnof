#include "../cui/api.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <vector>

class iObject {
public:
   virtual ~iObject() {}

   virtual void render(std::ostream& o) {}
   virtual void place(cui::pnt p) {}
};

class textObject : public iObject {
public:
   virtual void render(std::ostream& o) { o << payload; }

   std::string payload;
};

class controlObject : public iObject {
public:
   controlObject() : length(0), height(0), pnt(0,0) {}

   virtual void place(cui::pnt p) { pnt = p; }

   std::string baseType;
   std::string name;
   int length;
   int height;
   std::string format1;
   std::string format2;
   cui::pnt pnt;
};

class objectTable {
public:
   ~objectTable()
   {
      for(auto it = m_objects.begin();it!=m_objects.end();++it)
         for(auto *pO : it->second)
            delete pO;
   }

   std::list<iObject*>& create(size_t i)
   {
      std::list<iObject*>& l = m_objects[i];
      if(l.size())
         throw std::runtime_error("object multiply defined?");
      return l;
   }

   std::list<iObject*>& demand(size_t i)
   {
      std::list<iObject*>& l = m_objects[i];
      if(l.size() == 0)
         throw std::runtime_error("object undefined?");
      return l;
   }

   template<class T>
   void foreach(std::function<void(T&)> f)
   {
      for(auto it=m_objects.begin();it!=m_objects.end();++it)
      {
         for(auto *pObj : it->second)
         {
            T *pDObj = dynamic_cast<T*>(pObj);
            if(pDObj)
               f(*pDObj);
         }
      }
   }

private:
   std::map<size_t,std::list<iObject*> > m_objects;
};

void eatUntil(const char*& pThumb, char c)
{
   for(;*pThumb!=c;++pThumb);
   ++pThumb;
}

void parseObject(const char*& pThumb, std::list<iObject*>& list)
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
   else
   {
      std::stringstream stream;
      stream << "unknown object '" << pThumb << "'";
      throw std::runtime_error(stream.str());
   }
}

void parseObjectLine(const std::string& line, bool& stop, objectTable& oTable)
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

void generateFactory(const std::string& typeName, const std::string& cuiType, std::ostream& out)
{
   // generate the image factory
   out << std::endl;
   out << "class " << typeName << "_fac : public cui::plugInFactoryT<" << typeName << "," << cuiType << "> {" << std::endl;
   out << "public:" << std::endl;
   out << "   " << typeName << "_fac() : cui::plugInFactoryT<" << typeName << "," << cuiType << ">(\"" << typeName << "\") {}" << std::endl;
   out << "};" << std::endl;
   out << std::endl;
   out << "tcatExposeTypeAs(" << typeName << "_fac,cui::iPlugInFactory);" << std::endl;
}

int main(int argc, const char *argv[])
{
   std::cout << "{{" << std::endl;
   for(int i=0;i<argc;i++)
   {
      std::cout << "  " << argv[i] << std::endl;
   }
   std::cout << "}}" << std::endl;

   // load all the lines
   std::vector<std::string> lines;
   {
      std::ifstream in(argv[1]);
      while(in.good())
      {
         std::string line;
         std::getline(in,line);
         lines.push_back(line);
      }
   }

   // enforce ID
   if(::strncmp(lines[0].c_str(),"coml v1 ",8) != 0)
   {
      std::cerr << "bad coml header" << std::endl;
      return -1;
   }

   // read index
   int height = 0;
   ::sscanf(lines[0].c_str(),"coml v1 %d",&height);

   // compute name
   std::string name;
   {
      const char *pThumb = ::strstr(argv[1],"/screen.") + 8;
      if(!pThumb)
      {
         std::cerr << "bad coml file path" << std::endl;
         return -1;
      }
      name = std::string(pThumb,::strlen(pThumb) - 5);
   }

   // build object table
   objectTable oTable;
   for(size_t i=1+height;;i++)
   {
      if(i >= lines.size())
         break; // done
      bool stop = false;
      parseObjectLine(lines[i],stop,oTable);
      if(stop)
         break; // rest are comments
   }

   // generate
   std::ofstream out(argv[2]);
   out << "// *******************************" << std::endl;
   out << "// ** code generated by coml" << std::endl;
   out << "// *******************************" << std::endl;
   out << std::endl;
   out << "#include \"../../src/cmn/service.hpp\"" << std::endl;
   out << "#include \"../../src/cui/api.hpp\"" << std::endl;
   out << "#include \"../../src/cui/pen.hpp\"" << std::endl;
   out << "#include \"../../src/tcatlib/api.hpp\"" << std::endl;
   out << "#include <iostream>" << std::endl;

   // generate the image
   out << std::endl;
   out << "class " << name << "_image : public cui::iImage {" << std::endl;
   out << "public:" << std::endl;
   out << "   virtual void render()" << std::endl;
   out << "   {" << std::endl;
   out << "      tcat::typePtr<cmn::serviceManager> svcMan;" << std::endl;
   out << "      auto& pn = svcMan->demand<pen::object>();" << std::endl;
   out << std::endl;

   for(int i=0;i<height;i++)
   {
      out << "      pn.str() << \"";

      size_t jOffset = 0;
      auto& line = lines[i+1];
      for(size_t j=0;j!=line.length();j++)
      {
         if(::isdigit(line.c_str()[j]))
         {
            int id = 0;
            int n = 0;
            ::sscanf(line.c_str()+j,"%d%n",&id,&n);
            auto& objs = oTable.demand(id);
            for(auto *pObj : objs)
            {
               pObj->place(cui::pnt(j+1-jOffset,i+1));
               pObj->render(out);
            }
            jOffset += n;
            j += (n-1);
         }
         else if(line.c_str()[j] == '_')
            ; // ignore this char
         else
            out << std::string(1,line.c_str()[j]);
      }
      out << "\" << std::endl;" << std::endl;
   }

   out << "   }" << std::endl;
   out << std::endl;
   out << "   virtual cui::pnt demandPnt(const std::string& id) { throw 3.14; }" << std::endl;
   out << "};" << std::endl;

   // generate the image factory
   generateFactory(name + "_image","cui::iImage",out);

   // generate any control object(s)
   oTable.foreach<controlObject>([&](auto& ctl)
   {
      out << std::endl;
      out << "class " << name << "_" << ctl.name << "_ctl : public " << ctl.baseType << " {" << std::endl;
      out << "protected:" << std::endl;
      out << "   virtual void formatText1(std::ostream& o)" << std::endl;
      out << "   {" << std::endl;
      if(!ctl.format1.empty())
         out << "      o" << ctl.format1 << ";" << std::endl;
      out << "   }" << std::endl;
      out << "   virtual void formatText2(std::ostream& o)" << std::endl;
      out << "   {" << std::endl;
      if(!ctl.format2.empty())
         out << "      o" << ctl.format2 << ";" << std::endl;
      out << "   }" << std::endl;
      out << "};" << std::endl;
   });

   // generate the screen
   out << std::endl;
   out << "class " << name << "_screen : public cui::basicScreen {" << std::endl;
   out << "private:" << std::endl;
   out << "   " << name << "_image m_image;" << std::endl;
   oTable.foreach<controlObject>([&](auto& ctl)
   {
      out << "   " << name << "_" << ctl.name << "_ctl m_" << ctl.name << ";" << std::endl;
   });
   out << std::endl;
   out << "public:" << std::endl;
   out << "   " << name << "_screen()" << std::endl;
   out << "   {" << std::endl;
   out << "      publishObject(\"\",m_image);" << std::endl;
   oTable.foreach<controlObject>([&](auto& ctl)
   {
      out << "      publishObject(\"" << ctl.name << "\",m_" << ctl.name << ");" << std::endl;
   });
   out << "   }" << std::endl;
   out << std::endl;
   out << "   virtual void render()" << std::endl;
   out << "   {" << std::endl;
   out << "      m_image.render();" << std::endl;
   oTable.foreach<controlObject>([&](auto& ctl)
   {
      out << "      m_" << ctl.name << ".initialize(cui::pnt(" << ctl.pnt.x << "," << ctl.pnt.y << ")," << ctl.length << "," << ctl.height << ");" << std::endl;
   });
   out << "   }" << std::endl;
   out << "};" << std::endl;

   // generate the image factory
   generateFactory(name + "_screen","cui::iScreen",out);

   return 0;
}
