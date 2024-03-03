#ifndef __LINUX_HOOK_HH__
#define __LINUX_HOOK_HH__
#include "./hook.hh"

namespace NTNative {
#ifdef _WIN32
class WindowsHook: public Hook {
public:
  /**
   * @brief Construct a new Hook object
   *
   * @param name 模块名称
   *
   */
  WindowsHook(pid_t _pid, std::string &name) {
    pid = _pid;
    m_moduleName = name;
  };

protected:
  std::pair<unsigned long, unsigned long> get_module_address();
};
#endif

}

#endif