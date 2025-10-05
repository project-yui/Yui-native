
#include "napi.h"
#include "spdlog/spdlog.h"
#ifdef __linux__
#include <dlfcn.h>
#include <gnutls/gnutls.h>
#define YUI_EXPORT
#else
#include <windows.h>
#define YUI_EXPORT __declspec(dllexport)
#endif

extern "C" {
// Keep the existing registration helper
YUI_EXPORT void qq_magic_napi_register(napi_module *m) {
    spdlog::info("call qq_magic_napi_register");
    napi_module_register(m);
}
// On Windows the real napi_module_register will be provided by node/node.dll.
// We declare it here so it can be referenced inside extern "C"; the actual
// exported wrapper is implemented in C++ scope below for correct linkage.
#if defined(_WIN32) || defined(WIN32)
extern void napi_module_register(napi_module* m);
#endif
    
#if defined(__linux__) || defined(__APPLE__)
// Ensure gnutls_xxx is linked in
void * _force_gnutls_global_init __attribute__((used)) = (void *)gnutls_global_init;
#else

void * _force_gnutls_global_init = (void *)napi_module_register;
#endif
}

#ifdef WIN32

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    spdlog::info("DllMain !");
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        spdlog::info("DLL_PROCESS_ATTACH !");
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#endif

static Napi::Object Init(Napi::Env env, Napi::Object exports) {

  return exports;
}

NODE_API_MODULE(cmnative, Init)