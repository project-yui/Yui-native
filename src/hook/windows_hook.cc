#include "../include/windows_hook.hh"
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

namespace NTNative {

#ifdef _WIN32
  std::pair<unsigned long, unsigned long> WindowsHook::get_module_address() {
    // std::string maps_file = "/proc/" + std::to_string(pid) + "/maps";
    
    // std::ifstream file(maps_file);
    // std::pair<unsigned long, unsigned long> ret(0, 0);

    // if (!file.is_open()) {
    //   std::cerr << "Failed to open " << maps_file << std::endl;
    //   return ret;
    // }

    // std::string line;
    // while (std::getline(file, line)) {
    //   if (line.find(m_moduleName) == std::string::npos)
    //     continue;

    //   std::istringstream iss(line);
    //   std::string address_range, perms, offset, dev, inode, pathname;
    //   unsigned long start, end;

    //   iss >> address_range >> perms >> offset >> dev >> inode;
    //   std::getline(iss, pathname);

    //   // Remove leading spaces from pathname
    //   pathname.erase(0, pathname.find_first_not_of(" "));

    //   // Print module address and pathname
    //   // std::cout << "Address: " << address_range << ", Path: " << pathname <<
    //   // std::endl;

    //   sscanf(address_range.c_str(), "%lx-%lx", &start, &end);
    //   // std::cout << "start:" << std::hex << start << "; end:" << std::hex <<
    //   // end << std::endl;
    //   if (ret.first == 0) {
    //     // 第一次
    //     ret.first = start;
    //     ret.second = end;
    //   } else if (ret.second == start) {
    //     ret.second = end;
    //   }
    // }

    // file.close();
    std::pair<unsigned long, unsigned long> r;
    return r;
  }
#endif
} // namespace NTNative
