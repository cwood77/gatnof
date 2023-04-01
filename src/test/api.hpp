#ifndef ___test_api___
#define ___test_api___

#include "../tcatlib/api.hpp"

namespace test {

class iAsserter {
public:
   virtual void assertTrue(bool value) = 0;
};

class iTest {
public:
   virtual ~iTest() {}
   virtual const char *getName() const = 0;
   virtual void run(iAsserter& a) = 0;
};

#define testDefineTest(__name__) \
class __name__ ## _testClass : public test::iTest { \
public: \
   virtual const char *getName() const { return #__name__; } \
   virtual void run(test::iAsserter& a); \
}; \
tcatExposeTypeAs(__name__ ## _testClass, test::iTest); \
void __name__ ## _testClass::run(test::iAsserter& a)

} // namespace test

#endif // ___test_api___
