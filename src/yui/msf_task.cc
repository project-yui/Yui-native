#include "../include/native_msf.hh"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <ios>
#include <map>
#include <memory>
#include <ostream>
#include <queue>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <sstream>
#include <utility>
#include "../proto/search.pb.h"

namespace yui {
std::queue<CustomTaskPkg> task_queue;

std::shared_ptr<NTNative::Hook> msf_request_hooker;
typedef int (*_msf_request_hook_func)(void *_this, MsfReqPkg **pkg);
struct RecoveryData {
  Data *data;
  Data originalData;
};
/**
 * @brief recovery data for msf
 * 
 * seq -> [originalData, customData]
 * 
 */
std::map<long, RecoveryData> recovery_msf_data;
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
  spdlog::info("uin: {}", pkg->uin.data);
  spdlog::info("cmd: {}", pkg->cmdAndData->cmd.data);
  auto data = pkg->cmdAndData->data;
  spdlog::debug("data address: {} -> {}", (void*)data->dataStart, (void *)data->dataEnd);
  spdlog::debug("data size: {}", data->dataEnd - data->dataStart);
  std::stringstream ss;
  for (uint8_t *i = data->dataStart; i < data->dataEnd; i++) {
    ss << " 0x" << std::uppercase << std::setfill('0') << std::setw(2) <<  std::hex << static_cast<unsigned int>(*i);
  }
  ss << std::endl;
  spdlog::debug("data: {}", ss.str());

  if (strcmp(pkg->cmdAndData->cmd.data, "OidbSvcTrpcTcp.0x972_6") == 0)
  {
    // 1. search friend
    // 2. target uin: 1145141919810
    nt::search::Stranger stranger_search;
    auto data = pkg->cmdAndData->data;
    stranger_search.ParsePartialFromArray(data->dataStart, data->dataEnd - data->dataStart);
    std::string uin = stranger_search.body().targetuin();
    spdlog::debug("target uin: {}", uin.data());
    // 3. ok

    if (uin == "1145141919810")
    {
      spdlog::debug("queue size: {}", task_queue.size());
      if (task_queue.size() > 0)
      {
        auto customPkg = task_queue.front();
        task_queue.pop();
        spdlog::debug("copy uin...");
        strcpy_s(pkg->uin.data, customPkg.uin.c_str());
        pkg->uin.size = customPkg.uin.length() << 1;
        spdlog::debug("copy cmd...");
        strcpy_s(pkg->cmdAndData->cmd.data, customPkg.cmd.c_str());
        pkg->cmdAndData->cmd.size = customPkg.cmd.length() << 1;
        spdlog::debug("copy data...");
        uint8_t * t = (uint8_t *)malloc(customPkg.data.size());
        for (int i=0; i < customPkg.data.size(); i++) {
          t[i] = customPkg.data[i];
        }
        spdlog::debug("data address: {}", (void *)t);

        recovery_msf_data[pkg->seq] = {
          data,
          *data
        };

        spdlog::debug("original address: {} -> {}", (void*)data->dataStart, (void*)data->dataEnd);

        data->dataStart = t;
        data->dataEnd = t + customPkg.data.size();
        spdlog::debug("call original function...");
        int ret = func(_this, p);
        return ret;
      }
    }
  }

  // call original function
  spdlog::debug("func address: {}", (void *)func);
  spdlog::debug("call msf func ...");

  int ret = func(_this, p);
  spdlog::debug("msf result: {}", ret);
  return ret;
}

void msf_request_add(CustomTaskPkg pkg)
{
  task_queue.emplace(pkg);
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
  spdlog::debug("uin: {}", pkg->uin.data);
  spdlog::debug("cmd: {}", pkg->cmd.data);
  spdlog::debug("data start: {}, end: {}", (void*)pkg->data->dataStart, (void*)pkg->data->dataEnd);
  spdlog::debug("data size: {}", pkg->data->dataEnd - pkg->data->dataStart);
  spdlog::debug("try to find recovery data.");
  if (recovery_msf_data.find(pkg->seq) != recovery_msf_data.end())
  {
    auto rec = recovery_msf_data[pkg->seq];
    spdlog::debug("original address: {} -> {}", (void *)rec.originalData.dataStart, (void *)rec.originalData.dataEnd);
    free(rec.data->dataStart);
    *rec.data = rec.originalData;
    recovery_msf_data.erase(pkg->seq);
  }

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