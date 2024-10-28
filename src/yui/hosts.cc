
#include <cstdint>
#include <cstring>
#include <memory>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <string.h>
#include "../include/handle.hh"

namespace yui {
  std::shared_ptr<NTNative::Hook> hosts_hooker;
  typedef int (* _hosts_hook_func)(StrItem * desc, StrItem * domain, uint8_t type, IPData* ips);
  int hosts_hook(StrItem * desc, StrItem * domain, uint8_t type, IPData* ips)
  {
    // spdlog::info("hosts requet......");
    _hosts_hook_func func = (_hosts_hook_func)hosts_hooker->get_trampoline();
    
    spdlog::debug("start: {}, end: {}", (void*)ips->start, (void*)ips->end);
    spdlog::debug("desc: {}, damain: {}, type:{}, ips size: {}", desc->data, domain->data, type, ips->size());
    if (strcmp(domain->data, "gchat.qpic.cn") == 0 && ips->size() > 0) {
      auto target = ips->start[0];
      spdlog::debug("modify first ip: {}", target.ip.data);
      #ifdef _WIN32
      strcpy_s(target.ip.data, "127.0.0.1");
      target.ip.length = 10;
      #elif defined (__linux__)
      strcpy(target.ip.data, "127.0.0.1");
      target.ip.length = 20;
      #endif
      // target.port = 8085;
      ips->start[0] = target;
    }
    // 执行原来的调用
    spdlog::debug("func address: {}", (void*) func);
    spdlog::debug("call hosts func ...");
    // linux会崩溃
    int ret = func(desc, domain, type, ips);
    return ret;
  }
}