#ifndef __NATIVE_STMT_HH__
#define __NATIVE_STMT_HH__
#include <cstdint>
#include <memory>
#include <sqlite3.h>
#include <vector>

#include "../include/hook.hh"

namespace yui {
  extern const char * db_name;
  extern std::shared_ptr<NTNative::Hook> sqlit3_stmt_hooker;
  int sqlite3_stmt_hook(void*);

}
#endif