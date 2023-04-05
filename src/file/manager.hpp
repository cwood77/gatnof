#ifndef ___file_manager___
#define ___file_manager___

#include "api.hpp"
#include <typeinfo>

namespace console { class iLog; }

namespace file {

class iFile;

class iFileManager {
public:
   enum pathRoots {
      kAppData,
      kUserData,
      kProgramFiles32Bit,
      kProgramFiles64Bit,
      kExeAdjacent,
      kCurrentFolder
   };

   enum closeTypes {
      kReadOnly,       // will never save
      kDiscardOnClose, // plan to save later if no error
      kSaveOnClose,
      kDeleteAndTidyOnClose
   };

   virtual ~iFileManager() {}

   virtual const char *calculatePath(pathRoots root, const char *pathSuffix) const = 0;
   virtual void createAllFoldersForFile(const char *path, console::iLog& l, bool really) const = 0;
   virtual void createAllFoldersForFolder(const char *path, console::iLog& l, bool really) const = 0;
   virtual void deleteFolderAndContentsIf(const char *path, console::iLog& l, bool really) const = 0;
   virtual bool isFolder(const char *path) const = 0;
   virtual bool doesFileExist(const std::string& path) const = 0;

   template<class T>
   T& bindFile(pathRoots root,
      const char *pathSuffix,
      closeTypes onClose = kDiscardOnClose,
      const sst::iNodeFactory& f = sst::defNodeFactory())
   {
      std::string path = calculatePath(root,pathSuffix);
      return dynamic_cast<T&>(_bindFile(typeid(T).name(),path.c_str(),onClose,f));
   }

   template<class T>
   T& bindFile(const char *path,
      closeTypes onClose = kDiscardOnClose,
      const sst::iNodeFactory& f = sst::defNodeFactory())
   {
      return dynamic_cast<T&>(_bindFile(typeid(T).name(),path,onClose,f));
   }

   // implement close actions for any open files now
   virtual void flushAllOpen() = 0;

protected:
   virtual iFile& _bindFile(
      const char *fileType,
      const char *path,
      closeTypes onClose,
      const sst::iNodeFactory& f) = 0;
};

class iFile {
public:
   virtual ~iFile() {}
   virtual void release() = 0;

   virtual bool existed() const = 0;
   virtual void scheduleFor(iFileManager::closeTypes onClose) = 0;

   virtual void tie(console::iLog& l) = 0;

   virtual void flush() = 0;
};

class iSstFile : public virtual iFile {
public:
   virtual sst::dict& dict() = 0;
   virtual sst::dict *abdicate() = 0;
};

} // namespace file

#endif // ___file_manager___
