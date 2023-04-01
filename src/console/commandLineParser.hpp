#ifndef ___console_commandLineParser___
#define ___console_commandLineParser___

#include "arg.hpp"

namespace console {

class commandLineParser : public iCommandLineParser {
public:
   iCommandLineParser& addVerb(iArgPattern& v);

   iCommand *parse(int argc, const char *argv[]);

private:
   std::list<iArgPattern*> m_patterns;
};

} // namespace console

#endif // ___console_commandLineParser___
