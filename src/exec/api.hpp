#ifndef ___exec_api___
#define ___exec_api___

namespace console { class iLog; }

namespace exec {

class iProcessRunner {
public:
   virtual ~iProcessRunner() {}

   virtual void execute(const char *command, console::iLog& l, bool wait) = 0;
};

} // namespace exec

#endif // ___exec_api___
