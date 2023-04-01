#include <fstream>
#include <iostream>

int main(int argc, const char *argv[])
{
   std::cout << "{{" << std::endl;
   for(int i=0;i<argc;i++)
   {
      std::cout << "  " << argv[i] << std::endl;
   }
   std::cout << "}}" << std::endl;

   {
      std::ofstream out(argv[2]);
      out << "// unimpled" << std::endl;
   }

   return 0;
}
