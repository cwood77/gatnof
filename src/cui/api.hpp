#ifndef ___cui_api___
#define ___cui_api___

#include <functional>
#include <string>

namespace cui { 

// --------------- infrastructure

class iObject {
public:
   virtual ~iObject() {}
   virtual void release() = 0;
};

// used to locate images/screens
class iFactory {
public:
   virtual iObject& create(const std::string& id) = 0;
};

class iPlugInFactory {
public:
   virtual iObject *tryCreate(const std::string& id) = 0;
};

// --------------- simple atomics

class pnt {
public:
   pnt(size_t x, size_t y) : x(x), y(y) {}

   size_t x;
   size_t y;
};

// --------------- toolbox of controls, etc.

class iTextBox : public iObject {
public:
   virtual void initialize(pnt p, size_t l) = 0;
   virtual std::string get() = 0;
   virtual void set(const std::string& v) = 0;
   void access(std::function<void(std::string&)> f);
};

// --------------- bases of codegened specifics

class iImage : public iObject {
public:
   virtual void render() = 0;
   virtual pnt demandPnt(const std::string& id) = 0;
};

class iScreen {
public:
   virtual iObject& demand(const std::string& id) = 0;
};

} // namespace cui

#endif // ___cui_api___
