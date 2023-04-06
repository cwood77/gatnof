#ifndef ___coml_styler___
#define ___coml_styler___

namespace coml {

class objectTable;
class styleIr;

class styler {
public:
   explicit styler(styleIr& ir) : m_ir(ir) {}

   void apply(objectTable& objs)
   {
      applyDefaults(objs);
      //replaceVars(objs);
   }

private:
   void applyDefaults(objectTable& oTable);
   void applyDefault(controlObject& o, controlStyleDefault& d);

   void replaceVars(objectTable& oTable);
   void replaceVars(std::string& expr);

   styleIr& m_ir;
};

} // namespace coml

#endif // ___coml_styler___
