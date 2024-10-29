#ifndef __NT_HOOK_HH__
#define __NT_HOOK_HH__

#include "subhook.h"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#ifdef __linux__
#include <sched.h>
#endif
#ifdef _WIN32
#define pid_t int
#endif

namespace NTNative {
/**
 * @brief Hook
 */
class Hook {
public:
  Hook(){};
  
  /**
   * @brief Construct a new Hook object
   *
   * @param name 模块名称
   *
   */
  Hook(pid_t pid, std::string &name) : pid(pid), m_moduleName(name){};

  virtual ~Hook(){
  };

  /**
   * @brief Set the Signature object
   * 
   * @param signature 
   */
  virtual bool set_signature(const std::vector<uint8_t>& _signature);

  /**
   * @brief 安装新的函数
   * 
   * 已安装的不能再安装
   * 
   * @return true 
   * @return false 
   */
  virtual bool install(void * dest);

  /**
   * @brief 获取原始函数地址
   * 
   * @return void* 
   */
  void * get_trampoline();
  void* original_func;
  subhook::Hook hook;

protected:
  /**
   * @brief 获取函数的起始地址
   * 
   * @return void* 
   */
  virtual void * get_start_addr();
  
  /**
   * @brief 检查特征码是否匹配
   * 
   * @param data 
   * @return true 
   * @return false 
   */
  bool is_feature_code_matched(const uint8_t* data);

  /**
   * @brief 搜索特征码
   * 
   * @param data 
   * @param size 
   * @return const uint8_t* 
   */
  const uint8_t* search_feature_code(const uint8_t* data, size_t size);
  
  /**
   * @brief 获取模块的起始地址
   * 
   * @param pid 
   * @param targetModule 
   * @return std::pair<void *, void *> (startAddr, size)
   */
  virtual std::pair<void *, long> get_module_address() = 0;
  
  // 进程id
  pid_t pid = -1;
  // 模块名称
  std::string m_moduleName;
  // 特征值
  std::vector<uint8_t> signature;
};

} // namespace NTNative
#endif