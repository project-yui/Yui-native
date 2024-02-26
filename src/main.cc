#include "include/linux_hook.hh"
#include <iostream>
#include <memory>
#include <random>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sstream>
#include <vector>
#include <napi.h>
#include <subhook.h>

#include <sqlite3.h>


#define BUF_SIZE 1024
namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}

typedef int (*foo_func)(void * 
,void *
 ,void *  
 ,void *  
 ,void * 
 ,void *  
//  ,int 
);
subhook::Hook foo_hook;
// void *foo;
int execute(void * a1
, void * a2
, void *  a3
, void *  a4
, void *  a5
, void *  a6
// , int a7
) {
    std::string uuid = uuid::generate_uuid_v4();
    const char * u = uuid.c_str();
    printf("[%s]execute\n", u);
    try
    {
        // std::cout << "foo(" << (char *)a << "," << *(char **)b << "," << *(char **)c  << "," << (char *)d << ") called" << std::endl;
        std::cout << "stmt(..." << ") called" << std::endl;
        sqlite3_stmt *stmt = nullptr;
        stmt = (sqlite3_stmt *)a1;
        if (stmt != nullptr)
        {
            const char* sql = sqlite3_sql(stmt);
            
            // std::cout << "sql:" << sql << std::endl;
            printf("sql:%s\n", sql);
        }
    }
    catch(const std::exception& e)
    {
        std::cout << "oops! error:" << e.what() << '\n';
    }
    
//   int ret = ((foo_func)foo)(a1
// //   , a2
// //     , a3
// //     , a4
// //     , a5
// //     , a6
// //     , a7
//   );
// //   std::cout << "result:" << std::hex << ret << std::endl;
    foo_func fun = (foo_func)foo_hook.GetTrampoline();
    if (fun == nullptr) {
        printf("[%s]error nullptr!!!\n", u);
        return -1;
    }
    // sleep(5);
    // int ret = sqlite3_step((sqlite3_stmt *)a1);
    int ret = fun(a1
  , a2
    , a3
    , a4
    , a5
    , a6);
    printf("[%s]result: %d\n", u, ret);
    return ret;

}

// 特征码
// const std::vector<uint8_t> feature_code = { 0x41, 0x57, 0x41, 0x56, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x49, 0x89, 0xFF, 0x48, 0x8B, 0x02, 0x48, 0x89, 0x44, 0x24, 0x10, 0xC7, 0x44, 0x24, 0x28, 0x0B, 0x00, 0x00, 0x00, 0xF6, 0x01, 0x01, 0x74, 0x06, 0x48, 0x8B, 0x49, 0x10, 0xEB, 0x03, 0x48, 0xFF, 0xC1, 0x4C, 0x8D, 0x74, 0x24, 0x30, 0x48, 0x89, 0x4C, 0x24, 0x30, 0xC7, 0x44, 0x24, 0x48, 0x0B, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x06, 0x48, 0x8B, 0x56, 0x08, 0x48, 0x8D, 0x4C, 0x24, 0x10, 0x41, 0xB8, 0x02, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xFF, 0x48, 0x89, 0xC6, 0xE8, 0x37, 0x0F, 0x9E, 0x03, 0x8B, 0x44, 0x24, 0x48, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0x48, 0x39, 0xD8, 0x74, 0x12, 0x48, 0x8D, 0x0D, 0x2A, 0x82, 0xCB, 0x03, 0x48, 0x8D, 0x7C, 0x24, 0x08, 0x4C, 0x89, 0xF6, 0xFF, 0x14, 0xC1, 0xC7, 0x44, 0x24, 0x48, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x44, 0x24, 0x28, 0x48, 0x39, 0xD8, 0x74, 0x14, 0x48, 0x8D, 0x0D, 0x07, 0x82, 0xCB };
const std::vector<uint8_t> feature_code = { 0x55, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x53, 0x48, 0x83, 0xEC, 0x18, 0x48, 0x85, 0xFF, 0x0F, 0x84, 0x96, 0x03, 0x00, 0x00, 0x49, 0x89, 0xFC, 0x48, 0x8B, 0x07, 0x48, 0x85, 0xC0 };

std::shared_ptr<NTNative::LinuxHook> linuxHook;

void hook2() {
    std::cout << "Hello, world!\n";
    
    std::string target = "wrapper.node";

    // printf("pid of this process:%d\n", pid);
    // getNameByPid(pid, task_name);

    /*
    strcpy(task_name, argv[0]+2);
    printf("task name is %s\n", task_name);
    getPidByName(task_name);
    */
    pid_t p = getpid();
    printf("current pid:%d\n", p);
    linuxHook.reset(new NTNative::LinuxHook(p, target));

    linuxHook->set_signature(feature_code);

    linuxHook->install((void *)execute);
}
static Napi::Boolean test(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  hook2();
  return Napi::Boolean::New(env, true);
}
static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  // https://github.com/chrononeko/bugtracker/issues/7
  // freopen_s(reinterpret_cast<FILE **> stdout, "CONOUT$", "w", stdout);

  exports.Set(
      Napi::String::New(env, "test"),
      Napi::Function::New(env, test));

//   exports.Set(
//       Napi::String::New(env, "setPBPreprocessorForGzHook"),
//       Napi::Function::New(env, setPBPreprocessorForGzHook));

//   exports.Set(
//       Napi::String::New(env, "setHandlerForPokeHook"),
//       Napi::Function::New(env, setHandlerForPokeHook));

  return exports;
}

NODE_API_MODULE(cmnative, Init)