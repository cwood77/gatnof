#ifndef ___exec_scriptRunner___
#define ___exec_scriptRunner___

#include "api.hpp"

namespace exec {

class processRunner : public iProcessRunner {
public:
   virtual void execute(const char *command, console::iLog& l, bool wait);
};

} // namespace exec

#endif // ___exec_scriptRunner___
