// Minimal QQNT DLL used to forward/export symbols. Some exports are
// implemented as local wrappers that log and then forward the call to
// QQNT_original.dll via GetProcAddress so the linker can resolve them.

#include "node_api.h"
#include <windows.h>
#include <spdlog/spdlog.h>

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        spdlog::info("QQNT DLL_PROCESS_ATTACH");
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// 这个函数，node没有自带
// Wrapper for mangled C++ symbol: ?IsEnvironmentStopping@node@@YA_NPEAVIsolate@v8@@@Z
// Original signature: bool IsEnvironmentStopping(node::Isolate*) (approx)
extern "C" __declspec(dllexport) int My_IsEnvironmentStopping_node(void* isolate)
{
    spdlog::info("My_IsEnvironmentStopping_node called");
    return 0;
}
