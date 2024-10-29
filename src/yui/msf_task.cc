
#include "../include/native_msf.hh"
#include <iomanip>
#include <ios>
#include <memory>
#include <ostream>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <sstream>

namespace yui {
std::shared_ptr<NTNative::Hook> msf_request_hooker;
typedef int (*_msf_request_hook_func)(void *_this, MsfReqPkg **pkg);

/**
 * @brief MSF的请求拦截执行函数
 * 
 * @param _this 
 * @param pkg 
 * @return int 
 */
int msf_request_hook(void *_this, MsfReqPkg **p) {
  spdlog::info("msf requet......");
  _msf_request_hook_func func = (_msf_request_hook_func)msf_request_hooker->get_trampoline();
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


std::shared_ptr<NTNative::Hook> msf_response_hooker;
typedef int (*_msf_response_hook_func)(void *_this, MsfRespPkg **pkg, int);
/**
 * @brief MSF的响应拦截执行函数
 * 
 * @param _this 
 * @param pkg 
 * @return int 
 */
int msf_response_hook(void *_this, MsfRespPkg **p, int a3) {
  spdlog::info("msf response......");
  _msf_response_hook_func func = (_msf_response_hook_func)msf_response_hooker->get_trampoline();
  if (func == nullptr)
  {
    spdlog::info("msf response null");
    return -1;
  }
  auto pkg = *p;

  spdlog::debug("error code: {}", a3);
  spdlog::debug("seq: {}", pkg->seq);
  spdlog::debug("uin: {}", pkg->uin);
  spdlog::debug("data start: {}, end: {}", (void*)pkg->data->dataStart, (void*)pkg->data->dataEnd);
  spdlog::debug("data size: {}", pkg->data->dataEnd - pkg->data->dataStart);
  std::stringstream ss;
  for (uint8_t *i = pkg->data->dataStart; i < pkg->data->dataEnd; i++) {
    ss << " 0x" << std::uppercase << std::setfill('0') << std::setw(2) <<  std::hex << static_cast<unsigned int>(*i);
  }
  ss << std::endl;
  spdlog::debug("data: {}", ss.str());

  int ret = func(_this, p, a3);
  spdlog::debug("msf result: {}", ret);
  return ret;
}
} // namespace yui