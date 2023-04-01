inline sst::dict& subobject::config()
{
   if(m_pConfig)
      return *m_pConfig;
   else
      throw std::runtime_error("suboject::config() used before tied!");
}

inline console::iLog& subobject::log()
{
   if(m_pLog)
      return *m_pLog;
   else
      throw std::runtime_error("suboject::log() used before tied!");
}
