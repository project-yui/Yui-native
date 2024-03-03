#ifndef __HANDLE_HH__
#define __HANDLE_HH__
#include <map>
#include <memory>
#include <sqlite3.h>

#ifdef __linux__
#include "../include/linux_hook.hh"
#elif _WIN32
#include "../include/windows_hook.hh"
#endif

namespace yukihana {
  extern const char * db_name;
  #ifdef __linux__
  extern std::shared_ptr<NTNative::Hook> hook;
  #elif _WIN32
  extern std::shared_ptr<NTNative::WindowsHook> hook;
  #endif
  int execute(void* , void* , void* , void* , void* , void* );
}
#endif