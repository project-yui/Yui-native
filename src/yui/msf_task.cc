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
#include "../proto/communication.pb.h"


namespace yui {
std::queue<CustomTaskPkg> task_queue;

std::shared_ptr<NTNative::Hook> msf_request_hooker;
typedef int (*_msf_request_hook_func)(void *_this, MsfReqPkg **pkg);
struct RecoveryData {
  Data *data;
  Data originalData;
  std::promise<std::pair<void *, long>> * promise;
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
  spdlog::debug("msf request......");
  _msf_request_hook_func func = (_msf_request_hook_func)msf_request_hooker->get_trampoline();
  if (func == nullptr)
  {
    spdlog::info("msf request null");
    return -1;
  }
  auto pkg = *p;
  spdlog::debug("seq: {}", pkg->seq);
  spdlog::debug("uin: {}", pkg->uin.data);
  spdlog::debug("cmd: {}", pkg->cmdAndData->cmd.data);
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
    nt::communication::TcpBase request;
    auto data = pkg->cmdAndData->data;
    request.ParsePartialFromArray(data->dataStart, data->dataEnd - data->dataStart);
    nt::communication::StrangerSearchReq body;
    body.ParseFromString(request.body());

    spdlog::debug("command: {}", request.command());
    std::string uin = body.targetuin();
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
        #ifdef _WIN32
        strcpy_s(pkg->uin.data, customPkg.uin.c_str());
        #endif
        #ifdef __linux__
        strcpy(pkg->uin.data, customPkg.uin.c_str());
        #endif
        pkg->uin.size = customPkg.uin.length() << 1;
        spdlog::debug("copy cmd...");
        #ifdef _WIN32
        strcpy_s(pkg->cmdAndData->cmd.data, customPkg.cmd.c_str());
        #endif
        #ifdef __linux__
        strcpy(pkg->cmdAndData->cmd.data, customPkg.cmd.c_str());
        #endif
        pkg->cmdAndData->cmd.size = customPkg.cmd.length() << 1;
        spdlog::debug("copy data...");
        uint8_t * t = (uint8_t *)malloc(customPkg.data.size());
        for (int i=0; i < customPkg.data.size(); i++) {
          t[i] = customPkg.data[i];
        }
        spdlog::debug("data address: {}", (void *)t);

        recovery_msf_data[pkg->seq] = {
          data,
          *data,
          customPkg.promise,
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
  spdlog::debug("msf response......");
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
    auto size = pkg->data->dataEnd - pkg->data->dataStart;
    auto rd = new uint8_t[size];
    memcpy(rd, pkg->data->dataStart, size);
    rec.promise->set_value(std::make_pair<void*, long>(rd, long(size)));
    
    // =========================response peocess start===========================
    // 1. parse resp
    // length:665
    // nt::communication::TcpBase resp;
    // // 2. modify msg
    // resp.set_command(2418);
    // resp.set_subcommand(6);
    // resp.set_errorcode(0);
    // resp.set_errormsg("this is error msg");
    // // 3. repack

    // auto respBody = new nt::communication::StrangerSearchResp();
    // auto f1 = new nt::communication::StrangerSearchRespField1();
    // f1->set_field1(1000);
    // f1->set_field2("查找人");
    // f1->set_field4(114);
    // respBody->set_allocated_field1(f1);
    // respBody->set_field2(114);
    // uint8_t f3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // respBody->set_field3(f3, 15);
    // resp.set_body(respBody->SerializePartialAsString());
    // resp.clear_properties();

    // auto size = resp.ByteSizeLong();
    // uint8_t* result = new uint8_t[size];
    // resp.SerializePartialToArray(result, size);
    // Data backup = *pkg->data;
    // pkg->data->dataStart = result;
    // pkg->data->dataEnd = result + size;

    // std::stringstream ss;
    // for (uint8_t *i = pkg->data->dataStart; i < pkg->data->dataEnd; i++) {
    //   ss << " 0x" << std::uppercase << std::setfill('0') << std::setw(2) <<  std::hex << static_cast<unsigned int>(*i);
    // }
    // ss << std::endl;
    // spdlog::debug("data: {}", ss.str());
    
    // // 4. clean memory
    // int ret = func(_this, p, a3);
    // spdlog::debug("msf result: {}", ret);
    // *pkg->data = backup;
    // // delete [] result;
    // return ret;
  }

  // std::stringstream ss;
  // for (uint8_t *i = pkg->data->dataStart; i < pkg->data->dataEnd; i++) {
  //   ss << " 0x" << std::uppercase << std::setfill('0') << std::setw(2) <<  std::hex << static_cast<unsigned int>(*i);
  // }
  // ss << std::endl;
  // spdlog::debug("data: {}", ss.str());

  int ret = func(_this, p, a3);
  spdlog::debug("msf result: {}", ret);
  return ret;
}
} // namespace yui