#include "../include/native_hosts.hh"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#ifdef __linux__
#include "../include/linux_hook.hh"
#include <unistd.h>
#endif
#ifdef _WIN32
#include <process.h>
#include "../include/windows_hook.hh"
#endif
#include <vector>
#include <napi.h>
#include <subhook.h>
#include <sqlite3.h>
#include "spdlog/spdlog.h"

bool install_hosts_hook(std::string &name, std::vector<uint8_t> & feature_code) {
    spdlog::info("internal install hook");
    
    std::string target = name;
#ifdef __linux__
    pid_t p = getpid();
#endif
#ifdef _WIN32
    pid_t p = _getpid();
#endif
    spdlog::debug("current pid: {}\n", p);
    #ifdef __linux__
    yui::hosts_hooker.reset(new NTNative::LinuxHook(p, target));
    #endif
    #ifdef _WIN32
    yui::hosts_hooker.reset(new NTNative::WindowsHook(p, target));
    #endif

    spdlog::debug("set_signature\n");
    yui::hosts_hooker->set_signature(feature_code);

    spdlog::debug("install\n");
    return yui::hosts_hooker->install((void *)yui::hosts_hook);
}