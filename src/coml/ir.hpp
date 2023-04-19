#ifndef ___coml_ir___
#define ___coml_ir___

#include "../cui/api.hpp"
#include <list>
#include <map>
#include <stdexcept>

namespace coml {

class iObject {
public:
   virtual ~iObject() {}

   virtual void render(std::ostream& o, int& jOffset) {}
   virtual void place(cui::pnt p) {}
   virtual size_t getShadow() { return 0; }
};

class textObject : public iObject {
public:
   virtual void render(std::ostream& o, int&) { o << payload; }

   std::string payload;
};

class labelObject : public textObject {
public:
   virtual void render(std::ostream& o, int& jOffset)
   {
      textObject::render(o,jOffset);
      jOffset -= payload.length();
   }
};

class controlObject : public iObject {
public:
   controlObject()
   : length(0), height(0), pnt(0,0), isTableElt(false) {}

   virtual void place(cui::pnt p) { pnt = p; }
   virtual size_t getShadow() { return length; }

   std::string baseType;
   std::string name;
   int length;
   int height;
   std::string format1;
   std::string format2;
   std::string format3;
   std::string format4;
   cui::pnt pnt;
   bool isTableElt;
};

class buttonControlObject : public controlObject {
public:
   std::string face;
};

class listControlObject : public controlObject {
public:
   listControlObject() : rowHeight(0) {}

   virtual size_t getShadow() { return 0; }

   int rowHeight;
   std::vector<size_t> elts;
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

class controlStyleDefault {
public:
   std::string format1;
   std::string format2;
   std::string format3;
   std::string format4;
};

class styleIr {
public:
   std::map<std::string,std::string> vars;
   controlStyleDefault str;
   controlStyleDefault btn;
};

class ir {
public:
   ir() : height(0), yOffset(1) {}

   void computeName(const std::string& comlPath);

   std::string name;
   std::vector<std::string> lines;
   int height;
   int yOffset;
   objectTable oTable;
};

} // namespace coml

#endif // ___coml_ir___
