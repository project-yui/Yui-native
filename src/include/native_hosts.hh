#ifndef __NATIVE_HOSTS_HH__
#define __NATIVE_HOSTS_HH__
#include <cstdint>
#include <memory>
#include <sqlite3.h>
#include <vector>

#include "../include/hook.hh"

namespace yui {
  extern std::shared_ptr<NTNative::Hook> hosts_hooker;

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
  
}
#endif