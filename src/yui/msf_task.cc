
#include "../include/handle.hh"
#include <cstdio>
#include <iomanip>
#include <ios>
#include <memory>
#include <ostream>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <sstream>
#include <stdio.h>

namespace yui {
std::shared_ptr<NTNative::Hook> msf_hooker;
typedef int (*_msf_hook_func)(void *_this, MsfPkg **pkg);

/**
 * @brief MSF的拦截执行函数
 * 
 * @param _this 
 * @param pkg 
 * @return int 
 */
int msf_hook(void *_this, MsfPkg **p) {
  spdlog::info("msf requet......");
  _msf_hook_func func = (_msf_hook_func)msf_hooker->get_trampoline();
  if (func == nullptr)
  {
    spdlog::info("msf requet null");
    return -1;
  }
  auto pkg = *p;
  spdlog::info("seq: {}", pkg->seq);
  spdlog::info("uin: {}", pkg->uin);
  spdlog::info("cmd: {}", pkg->cmdAndData->cmd);
  auto data = pkg->cmdAndData->data;
  spdlog::debug("data start1: {}, end1: {}", (void*)data->dataStart, (void *)data->dataEnd);
  spdlog::debug("data size: {}", data->dataEnd - data->dataStart);
  std::stringstream ss;
  for (uint8_t *i = data->dataStart; i < data->dataEnd; i++) {
    ss << " 0x" << std::uppercase << std::setfill('0') << std::setw(2) <<  std::hex << static_cast<unsigned int>(*i);
  }
  ss << std::endl;
  spdlog::debug("data: {}", ss.str());

  // call original function
  spdlog::debug("func address: {}", (void *)func);
  spdlog::debug("call msf func ...");

  int ret = func(_this, p);
  spdlog::debug("msf result: {}", ret);
  return ret;
}
} // namespace yui