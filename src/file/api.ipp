inline str& str::operator=(const std::string& value)
{
   set(value);
   return *this;
}

inline void str::set(const std::string& value)
{
   m_value = value;
}

inline const std::string& str::get()
{
   return m_value;
}

inline mint& mint::operator=(const size_t& value)
{
   set(value);
   return *this;
}

inline void mint::set(const size_t& value)
{
   m_value = value;
}

inline const size_t& mint::get()
{
   return m_value;
}

inline std::string mint::toString() const
{
   std::stringstream stream;
   stream << m_value;
   return stream.str();
}

inline tf& tf::operator=(bool value)
{
   set(value);
   return *this;
}

inline void tf::set(bool value)
{
   m_value = value;
}

inline bool tf::get()
{
   return m_value;
}

inline std::string tf::toString() const
{
   std::stringstream stream;
   stream << m_value;
   return stream.str();
}

inline dict::~dict()
{
   auto it = m_value.begin();
   for(;it!=m_value.end();++it)
      delete it->second;
}

inline bool dict::has(const std::string& key)
{
   auto it = m_value.find(key);
   return it != m_value.end();
}

inline node& dict::operator[](const std::string& key)
{
   return *m_value[key];
}

inline void dict::replace(const std::string& key, node *pValue)
{
   node *pOld = m_value[key];
   if(pOld)
      delete pOld;
   m_value[key] = pValue;
}

inline array::~array()
{
   auto it = m_value.begin();
   for(;it!=m_value.end();++it)
      delete *it;
}

inline size_t array::size() const
{
   return m_value.size();
}

inline node& array::operator[](size_t index)
{
   return *m_value[index];
}

inline void array::erase(size_t index)
{
   node *pDead = m_value[index];
   delete pDead;
   m_value.erase(m_value.begin()+index);
}

inline void array::replace(size_t index, node *pValue)
{
   node *pOld = m_value[index];
   if(pOld)
      delete pOld;
   m_value[index] = pValue;
}

inline defNodeFactory::defNodeFactory()
: m_pRoot(NULL)
{
}

inline defNodeFactory::~defNodeFactory()
{
   delete m_pRoot;
}

inline void *defNodeFactory::createRootDictNode() const
{
   m_pRoot = new dict();
   return m_pRoot;
}

inline void defNodeFactory::releaseRootDictNode() const
{
   m_pRoot = NULL;
}

inline void *defNodeFactory::dict_add(void *pNode, types t, const std::string& key) const
{
   switch(t)
   {
      case iNodeFactory::kDict:
         return &((dict*)pNode)->add<dict>(key);
         break;
      case iNodeFactory::kArray:
         return &((dict*)pNode)->add<array>(key);
         break;
      case iNodeFactory::kStr:
         return &((dict*)pNode)->add<str>(key);
         break;
      case iNodeFactory::kMint:
         return &((dict*)pNode)->add<mint>(key);
         break;
      case iNodeFactory::kTf:
         return &((dict*)pNode)->add<tf>(key);
         break;
      default:
         throw std::runtime_error("unsupported type in dict_add");
   }
}

inline void *defNodeFactory::array_append(void *pNode, types t) const
{
   switch(t)
   {
      case iNodeFactory::kDict:
         return &((array*)pNode)->append<dict>();
         break;
      case iNodeFactory::kArray:
         return &((array*)pNode)->append<array>();
         break;
      case iNodeFactory::kStr:
         return &((array*)pNode)->append<str>();
         break;
      case iNodeFactory::kMint:
         return &((array*)pNode)->append<mint>();
         break;
      case iNodeFactory::kTf:
         return &((array*)pNode)->append<tf>();
         break;
      default:
         throw std::runtime_error("unsupported type in dict_add");
   }
}

inline void defNodeFactory::str_set(void *pNode, const std::string& value) const
{
   ((str*)pNode)->set(value);
}

inline void defNodeFactory::mint_set(void *pNode, size_t value) const
{
   ((mint*)pNode)->set(value);
}

inline void defNodeFactory::tf_set(void *pNode, bool value) const
{
   ((tf*)pNode)->set(value);
}
