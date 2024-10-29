#ifndef __HANDLE_HH__
#define __HANDLE_HH__
#include <cstdint>
#include <memory>
#include <sqlite3.h>
#include <vector>

#include "../include/hook.hh"

namespace yui {
  extern const char * db_name;
  extern std::shared_ptr<NTNative::Hook> sqlit3_stmt_hooker;
  extern std::shared_ptr<NTNative::Hook> hosts_hooker;
  extern std::shared_ptr<NTNative::Hook> msf_hooker;
  int sqlite3_stmt_hook(void*);

#ifdef _WIN32
  struct StrItem {
    char data[16] = "";
    size_t length = 0;
    size_t capacity = 0;
  };
#elif defined(__linux__)
  struct StrItem {
    uint8_t length;
    char data[23];
  };
#endif
  struct IPItem {
    StrItem ip;
    int port;
  };
  struct IPData {
    IPItem *start;
    IPItem *end;
    int size(){
      return end - start;
    };
  };
  /**
   * @brief 获取域名的ip
   * 
   * @param desc 描述
   * @param domain 域名
   * @param type 类型 0:ipv4/1:ipv6
   * @param ips ip列表
   * @return int 
   */
  int hosts_hook(StrItem * desc, StrItem * domain, uint8_t type, IPData* ips);
  
  struct Data{
    uint8_t *dataStart = nullptr;
    uint8_t *dataEnd = nullptr;
  };
  struct CmdAndData{
      char cmd[32] = "";
      Data *data = nullptr;
  };
  struct MsfPkg {
      CmdAndData *cmdAndData = nullptr;
      char pad1[24] = "";
      char uin[32] = "";
      long seq = 0;
  };
  /**
   * @brief 获取域名的ip
   * 
   * @param desc 描述
   * @param domain 域名
   * @param type 类型 0:ipv4/1:ipv6
   * @param ips ip列表
   * @return int 
   */
  int msf_hook(void * _this, MsfPkg ** pkg);
}
#endif