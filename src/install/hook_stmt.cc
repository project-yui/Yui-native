#include "../include/native_stmt.hh"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#ifdef __linux__
#include "include/linux_hook.hh"
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

/**
 * @brief 安装hook
 * 
 * @param feature_code 
 */
bool install_sqlite3_hook(std::string &name, std::vector<uint8_t> & feature_code) {
    spdlog::info("internal install hook");
    
    std::string target = name;
    // init();
    sqlite3_initialize();
    // printf("pid of this process:%d\n", pid);
    // getNameByPid(pid, task_name);

    /*
    strcpy(task_name, argv[0]+2);
    printf("task name is %s\n", task_name);
    getPidByName(task_name);
    */
#ifdef __linux__
    pid_t p = getpid();
#endif
#ifdef _WIN32
    pid_t p = _getpid();
#endif
    spdlog::debug("current pid: {}\n", p);
    #ifdef __linux__
    yui::sqlit3_stmt_hooker.reset(new NTNative::LinuxHook(p, target));
    #endif
    #ifdef _WIN32
    yui::sqlit3_stmt_hooker.reset(new NTNative::WindowsHook(p, target));
    #endif

    spdlog::debug("set_signature\n");
    yui::sqlit3_stmt_hooker->set_signature(feature_code);

    spdlog::debug("install\n");
    return yui::sqlit3_stmt_hooker->install((void *)yui::sqlite3_stmt_hook);
}