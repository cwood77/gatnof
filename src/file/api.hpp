#ifndef ___file_api___
#define ___file_api___

#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace sst {

// simple structed text

class node {
public:
   virtual ~node() {}

   template<class T>
   T& as() { return dynamic_cast<T&>(*this); }

   template<class T>
   T *is() { return dynamic_cast<T*>(this); }
};

class str : public node {
public:
   typedef std::string pod_t;

   str& operator=(const std::string& value);

   void set(const std::string& value);
   const std::string& get();

private:
   std::string m_value;
};

class mint : public node {
public:
   typedef size_t pod_t;

   mint& operator=(const size_t& value);

   void set(const size_t& value);
   const size_t& get();

   std::string toString() const;

private:
   size_t m_value;
};

class tf : public node {
public:
   typedef bool pod_t;

   tf& operator=(bool value);

   void set(bool value);
   bool get();

   std::string toString() const;

private:
   bool m_value;
};

class dict : public node {
public:
   virtual ~dict();

   bool has(const std::string& key);

   node& operator[](const std::string& key);

   template<class T>
   T& add(const std::string& key)
   { 
      T *pValue = new T();
      replace(key,pValue);
      return *pValue;
   }

   template<class T>
   typename T::pod_t getOpt(const std::string& key, const typename T::pod_t& defV)
   {
      auto it = m_value.find(key);
      if(it==m_value.end())
         return defV;
      return it->second->as<T>().get();
   }

   std::map<std::string,node*>& asMap() { return m_value; }

private:
   void replace(const std::string& key, node *pValue);

   std::map<std::string,node*> m_value;
};

class array : public node {
public:
   virtual ~array();

   size_t size() const;
   node& operator[](size_t index);

   template<class T>
   T& addAt(size_t index)
   { 
      T *pValue = new T();
      replace(index,pValue);
      return *pValue;
   }

   template<class T>
   T& append()
   {
      T *pValue = new T();
      m_value.push_back(pValue);
      return *pValue;
   }

   void erase(size_t index);

   std::vector<node*>& asVector() { return m_value; }

private:
   void replace(size_t index, node *pValue);

   std::vector<node*> m_value;
};

class iSerializer {
public:
   virtual ~iSerializer() {}
   virtual const char *write(node& n) = 0;
};

// provided by client so allocations happen in client
// memory-space
class iNodeFactory {
public:
   enum types {
      kDict,
      kArray,
      kStr,
      kMint,
      kTf
   };

   virtual void *createRootDictNode() const = 0;
   virtual void releaseRootDictNode() const = 0;

   virtual void *dict_add(void *pNode, types t, const std::string& key) const = 0;
   virtual void *array_append(void *pNode, types t) const = 0;

   virtual void str_set(void *pNode, const std::string& value) const = 0;
   virtual void mint_set(void *pNode, size_t value) const = 0;
   virtual void tf_set(void *pNode, bool value) const = 0;
};

class defNodeFactory : public iNodeFactory {
public:
   defNodeFactory();
   ~defNodeFactory();

   virtual void *createRootDictNode() const;
   virtual void releaseRootDictNode() const;
   virtual void *dict_add(void *pNode, types t, const std::string& key) const;
   virtual void *array_append(void *pNode, types t) const;
   virtual void str_set(void *pNode, const std::string& value) const;
   virtual void mint_set(void *pNode, size_t value) const;
   virtual void tf_set(void *pNode, bool value) const;

private:
   mutable dict *m_pRoot;
};

class iDeserializer {
public:
   virtual ~iDeserializer() {}
   virtual dict *parse(const char *pPtr, const iNodeFactory& f = defNodeFactory()) = 0;
};

#include "api.ipp"

} // namespace sst

#endif // ___file_api___
