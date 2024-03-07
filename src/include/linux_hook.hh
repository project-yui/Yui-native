#ifndef __LINUX_HOOK_HH__
#define __LINUX_HOOK_HH__
#ifdef __linux__
#include "./hook.hh"

namespace NTNative {
class LinuxHook: public Hook {
public:
  /**
   * @brief Construct a new Hook object
   *
   * @param name 模块名称
   *
   */
  LinuxHook(pid_t _pid, std::string &name) {
    pid = _pid;
    m_moduleName = name;
  };

protected:
  std::pair<void *, long> get_module_address();
};

}

#endif
#endif