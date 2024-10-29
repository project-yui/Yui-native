#ifndef __NATIVE_MSF_HH__
#define __NATIVE_MSF_HH__
#include <cstdint>
#include <memory>
#include <sqlite3.h>
#include <vector>

#include "../include/hook.hh"

namespace yui {

  struct Data{
    uint8_t *dataStart = nullptr;
    uint8_t *dataEnd = nullptr;
  };
  
  extern std::shared_ptr<NTNative::Hook> msf_request_hooker;
  struct CmdAndData{
      char cmd[32] = "";
      Data *data = nullptr;
  };
  /**
   * @brief 大小：0xD0,208
   * 
   */
  struct MsfReqPkg {
      CmdAndData *cmdAndData = nullptr;
      int offset_8 = 0;
      char pad1[20] = "";
      /**
       * @brief QQ
       * 
       * offset_32
       */
      char uin[32] = "";
      /**
       * @brief seq 
       * 
       * offset_64
       * 
       */
      long seq = 0;
      uint8_t pad2[18];
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
      uint8_t pad3[104];
  };
  /**
   * @brief MSF请求函数定义
   * 
   * @return int 
   */
  int msf_request_hook(void * _this, MsfReqPkg ** pkg);


  extern std::shared_ptr<NTNative::Hook> msf_response_hooker;
  struct MsfRespPkg {
    /**
     * @brief 占位
     * 
     */
    char uin[24];
    /**
     * @brief seq
     * 
     * offset 24
     * 
     */
    long seq;
    /**
     * @brief cmd
     * 
     * offset 32
     * 
     */
    char *cmd;
    /**
     * @brief 占位
     * 
     */
    uint8_t pad2[16];
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