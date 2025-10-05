
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
    // Example function to be exported
}

#ifdef WIN32

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    spdlog::info("QBar DllMain !");
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        spdlog::info("QBar DLL_PROCESS_ATTACH !");
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

#if defined(_WIN32) || defined(WIN32)
// Wrapper implementation for SetReaders
// We export a local symbol MySetReaders and map the mangled export to it in qbar.def

#include <mutex>

extern "C" {
    // Use C linkage for the wrapper symbol so .def can map to it
    // Signature assumptions:
    // - It's a non-static member function: int Decoder::SetReaders(Reader** , size_t)
    // - On x64 Windows, calling convention is the same for all functions (first args in RCX, RDX, ...)
    // We'll implement a generic pointer-based proxy: (void* This, void* readers, unsigned long long k)
    __declspec(dllexport) int MySetReaders(void* This, void* readers, unsigned long long k);
}

int MySetReaders(void* This, void* readers, unsigned long long k)
{
    static HMODULE orig = NULL;
    static void* fn_ptr = NULL;
    static std::once_flag flag;

    std::call_once(flag, [&]() {
        orig = LoadLibraryA("qbar.original.dll");
        if (orig) {
            // original mangled name
            const char* name = "?SetReaders@Decoder@qbar@@QEAAHPEAW4Reader@2@_K@Z";
            fn_ptr = (void*)GetProcAddress(orig, name);
            if (!fn_ptr) {
                spdlog::error("Failed to find original SetReaders symbol: {}", name);
            }
        } else {
            spdlog::error("Failed to load qbar.original.dll");
        }
    });

    spdlog::info("MySetReaders called: This={}, readers={}, k={}", This, readers, k);

    if (!fn_ptr) {
        spdlog::error("Original SetReaders not available, returning -1");
        return -1;
    }

#if defined(_WIN64)
    // On x64, calling convention is uniform. Cast to a callable function pointer that returns int
    using OrigFn = int(*)(void*, void*, unsigned long long);
    OrigFn origFn = (OrigFn)fn_ptr;
    int ret = origFn(This, readers, k);
    spdlog::info("MySetReaders forwarded, return {}", ret);
    return ret;
#else
    // On x86, the original is likely __thiscall; attempt to call via a compiler-specific calling convention
    using OrigFn = int(__thiscall *)(void*, void*, unsigned long long);
    OrigFn origFn = (OrigFn)fn_ptr;
    int ret = origFn(This, readers, k);
    spdlog::info("MySetReaders forwarded (x86), return {}", ret);
    return ret;
#endif
}

#endif
