inline stringParameter& stringParameter::optional(size_t offset)
{
   return *new stringParameter(offset,false);
}

inline stringParameter& stringParameter::required(size_t offset)
{
   return *new stringParameter(offset,true);
}

inline bool stringParameter::tryMatchWord(const std::string& word)
{
   char *pBytes = (char*)m_pCmd;
   std::string& field = *(std::string*)(pBytes + m_offset);
   field = word;
   m_read = true;
   return true;
}

inline void stringParameter::adjustPatterns(std::list<iArgPattern*>& list)
{
   std::list<iArgPattern*>::iterator it = list.begin();
   for(;it!=list.end();++it)
   {
      if(*it == this)
      {
         list.erase(it);
         return;
      }
   }
}

inline iCommand *stringParameter::complete()
{
   m_pVerb->verify();
   return m_pCmd;
}

inline void stringParameter::verify()
{
   if(!m_read && m_required)
      throw std::runtime_error("bad usage: missing parameter");
}

inline void stringParameter::collaborate(iCommand& c, iCommandVerifier& verb)
{
   m_pCmd = &c;
   m_pVerb = &verb;
}

inline stringParameter::stringParameter(size_t offset, bool required)
: m_offset(offset)
, m_required(required)
, m_pCmd(NULL)
, m_pVerb(NULL)
, m_read(false)
{
}

inline boolOption::boolOption(const std::string& tag, size_t offset)
: m_offset(offset)
, m_pCmd(NULL)
, m_pVerb(NULL)
{
   m_tags.insert(tag);
}

inline bool boolOption::tryMatchWord(const std::string& word)
{
   return m_tags.find(word) != m_tags.end();
}

inline void boolOption::adjustPatterns(std::list<iArgPattern*>& list)
{
   char *pBytes = (char*)m_pCmd;
   bool& field = *(bool*)(pBytes + m_offset);
   field = true;

   std::list<iArgPattern*>::iterator it = list.begin();
   for(;it!=list.end();++it)
   {
      if(*it == this)
      {
         list.erase(it);
         return;
      }
   }
}

inline iCommand *boolOption::complete()
{
   m_pVerb->verify();
   return m_pCmd;
}

inline void boolOption::collaborate(iCommand& c, iCommandVerifier& verb)
{
   m_pCmd = &c;
   m_pVerb = &verb;
}

inline iOption& boolOption::addTag(const std::string& tag)
{
   m_tags.insert(tag);
   return *this;
}

inline verbBase::~verbBase()
{
   {
      std::list<iCommandConfig*>::iterator it = m_options.begin();
      for(;it!=m_options.end();++it)
         delete *it;
   }
   {
      std::list<iCommandConfig*>::iterator it = m_params.begin();
      for(;it!=m_params.end();++it)
         delete *it;
   }
   delete m_pCmd;
}

inline verbBase& verbBase::addParameter(iCommandConfig& c)
{
   m_params.push_back(&c);
   c.collaborate(*m_pCmd,*this);
   return *this;
}

inline iOption& verbBase::addOption(iOption& o)
{
   m_options.push_back(&o);
   o.collaborate(*m_pCmd,*this);
   return o;
}

inline bool verbBase::tryMatchWord(const std::string& word)
{
   return word == m_tag;
}

inline void verbBase::adjustPatterns(std::list<iArgPattern*>& list)
{
   list.clear();

   {
      std::list<iCommandConfig*>::iterator it = m_options.begin();
      for(;it!=m_options.end();++it)
         list.push_back(*it);
   }
   {
      std::list<iCommandConfig*>::iterator it = m_params.begin();
      for(;it!=m_params.end();++it)
         list.push_back(*it);
   }
}

inline iCommand *verbBase::complete()
{
   verify();
   return m_pCmd;
}

inline void verbBase::verify()
{
   std::list<iCommandConfig*>::iterator it = m_params.begin();
   for(;it!=m_params.end();++it)
      (*it)->verify();
}

inline verbBase::verbBase(iCommand& c, const std::string& tag)
: m_pCmd(&c)
, m_tag(tag)
{
}

inline verbsGlobal& verbsGlobal::get()
{
   static verbsGlobal the;
   return the;
}

inline void verbsGlobal::add(iGlobalVerb& v)
{
   m_verbs.push_back(&v);
}

inline void verbsGlobal::program(iCommandLineParser& p)
{
   auto it = m_verbs.begin();
   for(;it!=m_verbs.end();++it)
      (*it)->program(p);
}

inline void verbsGlobal::deflate()
{
   auto it = m_verbs.begin();
   for(;it!=m_verbs.end();++it)
      (*it)->deflate();
}

inline globalVerb::globalVerb()
: m_pVerb(NULL)
{
   verbsGlobal::get().add(*this);
}

inline void globalVerb::program(iCommandLineParser& p)
{
   m_pVerb = inflate();
   p.addVerb(*m_pVerb);
}

inline void globalVerb::deflate()
{
   delete m_pVerb;
   m_pVerb = NULL;
}

inline autoVerbs::~autoVerbs()
{
   verbsGlobal::get().deflate();
}

inline void autoVerbs::program(iCommandLineParser& p)
{
   verbsGlobal::get().program(p);
}
