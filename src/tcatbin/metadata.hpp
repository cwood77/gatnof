#ifndef ___tcatbin_metadata___
#define ___tcatbin_metadata___

#include <map>
#include <set>
#include <string>

namespace tcatbin {

class iTypeServer;
class iModuleServer;
class libTable;

// lookup table for type -> iTypeServer
class catalogMetadata {
public:
   void record(iTypeServer& type);

   iTypeServer& demandOne(const std::string& typeName);
   std::set<iTypeServer*> getAll(const std::string& typeName);

private:
   std::map<std::string,std::set<iTypeServer*> > m_data;
};

// holds a DLL is memory while it's checked
class libProbe {
public:
   explicit libProbe(const std::string& filePath);
   ~libProbe();

   bool isLoaded() const;
   iModuleServer *getServer() const;

   void transfer(libTable& table);

private:
   void *m_pLib;
};

// examines a file for server-ness
class fileReflector {
public:
   fileReflector(catalogMetadata& data, libTable& libs);

   void reflectFile(const std::string& candidatePath);

private:
   catalogMetadata& m_meta;
   libTable& m_libs;
};

// walks files
class folderReflector {
public:
   explicit folderReflector(fileReflector& reflector);

   void reflectFolder(const std::string& folder);

private:
   fileReflector& m_fileReflector;
};

} // namespace tcatbin

#endif // ___tcatbin_metadata___
