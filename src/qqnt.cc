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

// Generic forwarding helpers. For functions with unknown signatures we
// implement no-arg wrappers and for qq_magic_napi_register we preserve
// the single-argument form used elsewhere in the project.

extern "C" {

} // extern "C"

// Additional forwarding wrappers for N-API and libuv functions imported from QQNT.dll
extern "C" {

// 这个函数，node没有自带
// Wrapper for mangled C++ symbol: ?IsEnvironmentStopping@node@@YA_NPEAVIsolate@v8@@@Z
// Original signature: bool IsEnvironmentStopping(node::Isolate*) (approx)
extern "C" __declspec(dllexport) int My_IsEnvironmentStopping_node(void* isolate)
{
    return 0;
}

} // extern "C"

