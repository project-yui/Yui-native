#ifndef __HANDLE_HH__
#define __HANDLE_HH__
#include <memory>
#include <sqlite3.h>

#include "../include/hook.hh"

namespace yukihana {
  extern const char * db_name;
  extern std::shared_ptr<NTNative::Hook> hook;
  int execute(void* , void* , void* , void* , void* , void* );
}
#endif