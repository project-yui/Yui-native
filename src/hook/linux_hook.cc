#include "../include/linux_hook.hh"
#include <fstream>
#include <iostream>
#include <sstream>

namespace NTNative {

#ifdef __linux__
  std::pair<void *, long> LinuxHook::get_module_address() {
    std::string maps_file = "/proc/" + std::to_string(pid) + "/maps";
    
    std::ifstream file(maps_file);
    std::pair<void *, long> ret(0, 0);

    if (!file.is_open()) {
      std::cerr << "Failed to open " << maps_file << std::endl;
      return ret;
    }

    std::string line;

    unsigned long long _start = 0, _end = 0;
    while (std::getline(file, line)) {
      if (line.find(m_moduleName) == std::string::npos)
        continue;

      std::istringstream iss(line);
      std::string address_range, perms, offset, dev, inode, pathname;
      unsigned long long start, end;

      iss >> address_range >> perms >> offset >> dev >> inode;
      std::getline(iss, pathname);

      // Remove leading spaces from pathname
      pathname.erase(0, pathname.find_first_not_of(" "));

      // Print module address and pathname
      // std::cout << "Address: " << address_range << ", Path: " << pathname <<
      // std::endl;

      sscanf(address_range.c_str(), "%llx-%llx", &start, &end);
      // std::cout << "start:" << std::hex << start << "; end:" << std::hex <<
      // end << std::endl;
      if (_start == 0) {
        // 第一次
        _start = start;
        _end = end;
      } else if (_end == start) {
        _end = end;
      }
    }
    ret.first = (void *)_start;
    ret.second = _end - _start;
    file.close();
    return ret;
  }
#endif
} // namespace NTNative
