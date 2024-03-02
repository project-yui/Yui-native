#ifndef __HANDLE_HH__
#define __HANDLE_HH__
#include <map>
#include <memory>
#include <sqlite3.h>
#include "../include/linux_hook.hh"

namespace yukihana {
  extern const char * db_name;
  extern std::shared_ptr<NTNative::LinuxHook> hook;
  extern std::map<void *, sqlite3_stmt *> nt2custom;
  int execute(void* , void* , void* , void* , void* , void* );
}
#endif