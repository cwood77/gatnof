// *******************************
// ** code generated by coml
// *******************************

#include "../../bin/out/debug/screen.test.hpp"

class test_image_fac : public cui::plugInFactoryT<test_image,cui::iImage> {
public:
   test_image_fac() : cui::plugInFactoryT<test_image,cui::iImage>("test_image") {}
};

tcatExposeTypeAs(test_image_fac,cui::iPlugInFactory);

class test_screen_fac : public cui::plugInFactoryT<test_screen,cui::iScreen> {
public:
   test_screen_fac() : cui::plugInFactoryT<test_screen,cui::iScreen>("test_screen") {}
};

tcatExposeTypeAs(test_screen_fac,cui::iPlugInFactory);
