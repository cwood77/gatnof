#include "metadata.hpp"
#include "api.hpp"
#include "tables.hpp"
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tcatbin {

void catalogMetadata::record(iTypeServer& type)
{
   m_data[type.getTypeName()].insert(&type);
}

iTypeServer& catalogMetadata::demandOne(const std::string& typeName)
{
   std::map<std::string,std::set<iTypeServer*> >::iterator it = m_data.find(typeName);
   if(it == m_data.end())
      throw std::runtime_error("type not found");
   if(it->second.size() != 1)
      throw std::runtime_error("too many types provided");
   return **(it->second.begin());
}

std::set<iTypeServer*> catalogMetadata::getAll(const std::string& typeName)
{
   std::map<std::string,std::set<iTypeServer*> >::iterator it = m_data.find(typeName);
   if(it == m_data.end())
      return std::set<iTypeServer*>();
   else
      return it->second;
}

libProbe::libProbe(const std::string& filePath)
: m_pLib(NULL)
{
   m_pLib = ::LoadLibrary(filePath.c_str());
}

libProbe::~libProbe()
{
   if(isLoaded())
      ::FreeLibrary((HMODULE)m_pLib);
}

bool libProbe::isLoaded() const
{
   return m_pLib != NULL;
}

iModuleServer *libProbe::getServer() const
{
   typedef tcatbin::iModuleServer& (*getFunc_t)();
   getFunc_t func = (getFunc_t)::GetProcAddress((HMODULE)m_pLib,"_Z15getModuleServerv");
   if(!func)
      return NULL;
   return &func();
}

void libProbe::transfer(libTable& table)
{
   table.add(m_pLib);
   m_pLib = NULL;
}

fileReflector::fileReflector(catalogMetadata& data, libTable& libs)
: m_meta(data), m_libs(libs)
{
}

void fileReflector::reflectFile(const std::string& candidatePath)
{
   if(candidatePath.length() > 4
      && ::strcmp(".dll",candidatePath.c_str()+candidatePath.length()-4)!=0)
      return;

   libProbe probe(candidatePath);
   if(!probe.isLoaded())
      return;

   iModuleServer *pSvr = probe.getServer();
   if(!pSvr)
      return;

   size_t n = pSvr->getNumTypes();
   for(size_t i=0;i<n;i++)
      m_meta.record(pSvr->getIthType(i));

   probe.transfer(m_libs);
}

folderReflector::folderReflector(fileReflector& reflector)
: m_fileReflector(reflector)
{
}

void folderReflector::reflectFolder(const std::string& folder)
{
   std::string pattern = folder;
   pattern += "\\*";

   WIN32_FIND_DATA fData;
   HANDLE hFind = ::FindFirstFileA(pattern.c_str(),&fData);
   if(hFind == INVALID_HANDLE_VALUE)
      throw std::runtime_error("error building catalog");
   do
   {
      m_fileReflector.reflectFile(folder + "\\" + fData.cFileName);
   }
   while (::FindNextFile(hFind,&fData));
   ::FindClose(hFind);
}

} // namespace tcatbin
