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

// Helper to load the original module and lookup an exported name.
static HMODULE get_orig()
{
    static HMODULE h = NULL;
    if (!h) {
        h = LoadLibraryA("QQNT.original.dll");
        if (!h) spdlog::error("Failed to LoadLibrary QQNT.original.dll");
    }
    return h;
}

// Generic forwarding helpers. For functions with unknown signatures we
// implement no-arg wrappers and for qq_magic_napi_register we preserve
// the single-argument form used elsewhere in the project.

extern "C" {

// ExportedContentMain : assume no-args for forwarding wrapper
__declspec(dllexport) void MyExportedContentMain()
{
    spdlog::info("MyExportedContentMain called");
    HMODULE h = get_orig();
    if (!h) return;
    auto p = (FARPROC)GetProcAddress(h, "ExportedContentMain");
    if (p) {
        using Fn = void(*)();
        ((Fn)p)();
    } else {
        spdlog::error("Original ExportedContentMain not found");
    }
}

// NodeContextifyContextMetrics1 : wrapper (no-args)
__declspec(dllexport) void MyNodeContextifyContextMetrics1()
{
    spdlog::info("MyNodeContextifyContextMetrics1 called");
    HMODULE h = get_orig();
    if (!h) return;
    auto p = (FARPROC)GetProcAddress(h, "NodeContextifyContextMetrics1");
    if (p) {
        using Fn = void(*)();
        ((Fn)p)();
    } else {
        spdlog::error("Original NodeContextifyContextMetrics1 not found");
    }
}

// PerfTrace : wrapper (no-args)
__declspec(dllexport) void MyPerfTrace()
{
    spdlog::info("MyPerfTrace called");
    HMODULE h = get_orig();
    if (!h) return;
    auto p = (FARPROC)GetProcAddress(h, "PerfTrace");
    if (p) {
        using Fn = void(*)();
        ((Fn)p)();
    } else {
        spdlog::error("Original PerfTrace not found");
    }
}

// QQMain : wrapper (no-args)
__declspec(dllexport) void MyQQMain()
{
    spdlog::info("MyQQMain called");
    HMODULE h = get_orig();
    if (!h) return;
    auto p = (FARPROC)GetProcAddress(h, "QQMain");
    if (p) {
        using Fn = void(*)();
        ((Fn)p)();
    } else {
        spdlog::error("Original QQMain not found");
    }
}

// qq_magic_napi_register : signature used elsewhere is void qq_magic_napi_register(napi_module*)
// we'll accept a void* and forward it.
__declspec(dllexport) void My_qq_magic_napi_register(napi_module* m)
{
    spdlog::info("My_qq_magic_napi_register called");
    napi_module_register(m);
}

} // extern "C"

