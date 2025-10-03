#ifndef __NATIVE_MSF_HH__
#define __NATIVE_MSF_HH__
#include <cstdint>
#include <future>
#include <memory>
#include <utility>
#include <vector>
#include <napi.h>

#include "../include/hook.hh"

namespace yui {

  struct Data{
    uint8_t *dataStart = nullptr;
    uint8_t *dataEnd = nullptr;
  };
  
  extern std::shared_ptr<NTNative::Hook> msf_request_hooker;
  struct NTLongStr {
    /**
     * @brief 长度左移一位
     * 
     */
    uint8_t size;
    char pad[7];
    int64_t realSize;
    char *pStr;
  };
  struct NTShortStr {
    /**
     * @brief 长度左移一位
     * 
     */
    uint8_t size;
    char data[23];
  };
  union NTStr {
    NTLongStr longStr;
    NTShortStr shortStr;
  };
  struct CmdAndData{
      NTStr cmd;
      uint8_t pad[8];
      /**
       * @brief
       * 
       * offset 32
       * 
       */
      Data *data = nullptr;
  };
  /**
   * @brief 大小：0xD0,208
   * 
   */
  struct MsfReqPkg {
      CmdAndData *cmdAndData = nullptr;
      int offset_8 = 32;
      char pad1[44] = "";
      /**
       * @brief QQ
       * 
       * offset_56
       */
      NTStr uin;
      uint8_t pad2[8] = {0};
      /**
       * @brief seq 
       * 
       * offset_88
       * 
       */
      long seq = 0;
      uint8_t pad3[58] = {0};
      /**
       * @brief uinType 
       * 
       * offset_160
       * 
       */
      int uinType = 0;
      /**
       * @brief 一个函数
       * 
       */
      void * offset_72 = nullptr;
      /**
       * @brief 一个函数
       * 
       */
      void * offset_80 = nullptr;
      /**
       * @brief 一个函数
       * 
       */
      void * offset_88 = nullptr;
      /**
       * @brief 一个函数
       * 
       */
      void * offset_96 = nullptr;
      /**
       * @brief 一个函数
       * 
       */
      void * offset_104 = nullptr;
      uint8_t pad4[96] = {0};
  };
  /**
   * @brief MSF请求函数定义
   * 
   * @return int 
   */
  int msf_request_hook(void * _this, MsfReqPkg ** pkg);
struct CustomTaskPkg {
  std::string cmd;
  std::string uin;
  std::vector<uint8_t> data;
  std::promise<std::pair<void *, long>> * promise;
};
  void msf_request_add(CustomTaskPkg pkg);


  extern std::shared_ptr<NTNative::Hook> msf_response_hooker;
  struct MsfRespPkg {
    /**
     * @brief 占位
     * 
     */
    NTStr uin;
    /**
     * @brief seq
     * 
     * offset 24
     * 
     */
    long seq;
    #ifdef _WIN32
    uint8_t pad[4] = {0};
    #endif
    /**
     * @brief cmd
     * 
     * offset 32
     * 
     */
    NTStr cmd;
    /**
     * @brief 数据
     *
     * offset 56
     * 
     */
    Data *data = nullptr;
  };
  /**
   * @brief MSF响应函数定义
   * 
   * @return int 
   */
  int msf_response_hook(void * _this, MsfRespPkg ** pkg, int);
}
#endif