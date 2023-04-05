#ifndef ___cmn_win32___
#define ___cmn_win32___

#define WIN32_LEAN_AND_MEAN
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <windows.h>

namespace cmn {

class osEvent {
public:
   explicit osEvent(const std::string& name = "", bool autoEvent = true)
   {
      m_hEvent = ::CreateEventA(NULL,!autoEvent,FALSE,name.empty() ? NULL : name.c_str());
   }

   ~osEvent()
   {
      ::CloseHandle(m_hEvent);
   }

   void set()
   {
      ::SetEvent(m_hEvent);
   }

   bool isSet()
   {
      DWORD r = ::WaitForSingleObject(m_hEvent,0);
      return r == WAIT_OBJECT_0;
   }

   void waitAndThrowIfTimeout(DWORD timeout, const std::string& msg = "timeout waiting for signal")
   {
      DWORD r = ::WaitForSingleObject(m_hEvent,timeout);
      if(r != WAIT_OBJECT_0)
         throw std::runtime_error(msg);
   }

   void waitForever(osEvent& stopSignal, bool& wasStopped)
   {
      HANDLE hans[2];
      hans[0] = m_hEvent;
      hans[1] = stopSignal.m_hEvent;
      DWORD r = ::WaitForMultipleObjects(2,hans,/*bAll*/FALSE,INFINITE);
      wasStopped = (r == (WAIT_OBJECT_0 + 1));
   }

   HANDLE getHandle() const { return m_hEvent; }

private:
   HANDLE m_hEvent;
};

class mutex {
public:
   mutex() : m_h(::CreateMutex(NULL,FALSE,NULL)) {}
   ~mutex() { ::CloseHandle(m_h); }
   void lock() { ::WaitForSingleObject(m_h,INFINITE); }
   void unlock() { ::ReleaseMutex(m_h); }

private:
   HANDLE m_h;
};

class autoLock {
public:
   explicit autoLock(mutex& m) : m_m(m) { m_m.lock(); }
   ~autoLock() { m_m.unlock(); }

private:
   mutex& m_m;
};

class iThread {
public:
   virtual void run() = 0;
};

class threadController {
public:
   explicit threadController(iThread& t) : m_thrd(t) {}

   void start()
   {
      m_hThread = ::CreateThread(
         NULL,
         0,
         &threadController::thunk,
         (LPVOID)this,
         0,
         NULL);
      if(m_hThread == INVALID_HANDLE_VALUE)
         throw std::runtime_error("failed to create thread");
   }

   void join()
   {
      ::WaitForSingleObject(m_hThread,INFINITE);
   }

private:
   static DWORD WINAPI thunk(LPVOID param)
   {
      try
      {
         auto *pSelf = (threadController*)param;
         pSelf->m_thrd.run();
         return 0;
      }
      catch(std::exception& x)
      {
         ::printf("ERROR: %s\r\n",x.what());
         return -3;
      }
      catch(...)
      {
         return -2;
      }
   }

   iThread& m_thrd;
   HANDLE m_hThread;
};

template<class T>
class threadGroup {
public:
   T& allocate()
   {
      m_members.push_back(new poolMember());
      auto *pPm = m_members.back();
      m_table[&pPm->thread] = pPm;
      return pPm->thread;
   }

   void run(T& t)
   {
      m_table[&t]->tc.start();
   }

   void join()
   {
      for(auto *pPm : m_members)
      {
         pPm->tc.join();
         delete pPm;
      }
   }

private:
   class poolMember {
   public:
      poolMember() : tc(thread) {}

      T thread;
      threadController tc;
   };

   std::list<poolMember*> m_members;
   std::map<T*,poolMember*> m_table;
};

class autoFindHandle {
public:
   autoFindHandle() : h(INVALID_HANDLE_VALUE) {}

   ~autoFindHandle() { ::FindClose(h); }

   HANDLE h;
};

} // namespace cmn

#endif // ___cmn_win32___
