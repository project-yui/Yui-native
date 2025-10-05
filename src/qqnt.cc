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
static HMODULE get_node()
{
    static HMODULE h = NULL;
    if (!h) {
        h = LoadLibraryA("node.exe");
        if (!h) spdlog::error("Failed to LoadLibrary node.exe");
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
__declspec(dllexport) void My_napi_module_register(napi_module* m)
{
    spdlog::info("My_napi_module_register called");
    // call from node.exe
    HMODULE h = get_node();
    if (!h) {
        spdlog::error("Failed to get node.exe handle");
        return;
    }

    spdlog::info("node.exe handle: {}", (void*)h);
    auto p = (FARPROC)GetProcAddress(h, "napi_module_register");
    if (p) {
        spdlog::info("Found original napi_module_register at {}", (void*)p);
        using Fn = void(*)(napi_module*);
        ((Fn)p)(m);
        spdlog::info("Called original napi_module_register");
    } else {
        spdlog::error("Original napi_module_register not found");
    }

}
// qq_magic_napi_register : signature used elsewhere is void qq_magic_napi_register(napi_module*)
// we'll accept a void* and forward it.
__declspec(dllexport) void My_qq_magic_napi_register(napi_module* m)
{
    spdlog::info("My_qq_magic_napi_register called");
    napi_module_register(m);
}
// qq_magic_napi_register : signature used elsewhere is void qq_magic_napi_register(napi_module*)
// we'll accept a void* and forward it.
__declspec(dllexport) void My_qq_magic_node_register(napi_module* m)
{
    spdlog::info("My_qq_magic_node_register called");
    napi_module_register(m);
}

} // extern "C"

// Additional forwarding wrappers for N-API and libuv functions imported from QQNT.dll
extern "C" {

__declspec(dllexport) napi_status My_napi_call_function(napi_env env, napi_value recv, napi_value func, size_t argc, const napi_value* argv, napi_value* result)
{
    spdlog::info("My_napi_call_function called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_call_function");
    if (!p) { spdlog::error("Original napi_call_function not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value,napi_value,size_t,const napi_value*,napi_value*);
    return ((Fn)p)(env, recv, func, argc, argv, result);
}

__declspec(dllexport) napi_status My_napi_throw_error(napi_env env, const char* code, const char* msg)
{
    spdlog::info("My_napi_throw_error called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_throw_error");
    if (!p) { spdlog::error("Original napi_throw_error not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,const char*,const char*);
    return ((Fn)p)(env, code, msg);
}

__declspec(dllexport) napi_status My_napi_create_string_utf8(napi_env env, const char* str, size_t length, napi_value* result)
{
    spdlog::info("My_napi_create_string_utf8 called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_create_string_utf8");
    if (!p) { spdlog::error("Original napi_create_string_utf8 not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,const char*,size_t,napi_value*);
    return ((Fn)p)(env, str, length, result);
}

__declspec(dllexport) napi_status My_napi_get_value_string_utf8(napi_env env, napi_value value, char* buf, size_t bufsize, size_t* result)
{
    spdlog::info("My_napi_get_value_string_utf8 called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_get_value_string_utf8");
    if (!p) { spdlog::error("Original napi_get_value_string_utf8 not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value,char*,size_t,size_t*);
    return ((Fn)p)(env, value, buf, bufsize, result);
}

__declspec(dllexport) napi_status My_napi_create_object(napi_env env, napi_value* result)
{
    spdlog::info("My_napi_create_object called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_create_object");
    if (!p) { spdlog::error("Original napi_create_object not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value*);
    return ((Fn)p)(env, result);
}

__declspec(dllexport) napi_status My_napi_create_double(napi_env env, double val, napi_value* result)
{
    spdlog::info("My_napi_create_double called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_create_double");
    if (!p) { spdlog::error("Original napi_create_double not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,double,napi_value*);
    return ((Fn)p)(env, val, result);
}

__declspec(dllexport) napi_status My_napi_create_int32(napi_env env, int32_t val, napi_value* result)
{
    spdlog::info("My_napi_create_int32 called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_create_int32");
    if (!p) { spdlog::error("Original napi_create_int32 not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,int32_t,napi_value*);
    return ((Fn)p)(env, val, result);
}

__declspec(dllexport) napi_status My_napi_create_uint32(napi_env env, uint32_t val, napi_value* result)
{
    spdlog::info("My_napi_create_uint32 called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_create_uint32");
    if (!p) { spdlog::error("Original napi_create_uint32 not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,uint32_t,napi_value*);
    return ((Fn)p)(env, val, result);
}

__declspec(dllexport) napi_status My_napi_create_array_with_length(napi_env env, size_t length, napi_value* result)
{
    spdlog::info("My_napi_create_array_with_length called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_create_array_with_length");
    if (!p) { spdlog::error("Original napi_create_array_with_length not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,size_t,napi_value*);
    return ((Fn)p)(env, length, result);
}

__declspec(dllexport) napi_status My_napi_create_arraybuffer(napi_env env, size_t size, void** data, napi_value* result)
{
    spdlog::info("My_napi_create_arraybuffer called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_create_arraybuffer");
    if (!p) { spdlog::error("Original napi_create_arraybuffer not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,size_t,void**,napi_value*);
    return ((Fn)p)(env, size, data, result);
}

__declspec(dllexport) napi_status My_napi_get_array_length(napi_env env, napi_value value, uint32_t* result)
{
    spdlog::info("My_napi_get_array_length called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_get_array_length");
    if (!p) { spdlog::error("Original napi_get_array_length not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value,uint32_t*);
    return ((Fn)p)(env, value, result);
}

__declspec(dllexport) napi_status My_napi_get_global(napi_env env, napi_value* result)
{
    spdlog::info("My_napi_get_global called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_get_global");
    if (!p) { spdlog::error("Original napi_get_global not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value*);
    return ((Fn)p)(env, result);
}

__declspec(dllexport) napi_status My_napi_get_named_property(napi_env env, napi_value object, const char* utf8Name, napi_value* result)
{
    spdlog::info("My_napi_get_named_property called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_get_named_property");
    if (!p) { spdlog::error("Original napi_get_named_property not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value,const char*,napi_value*);
    return ((Fn)p)(env, object, utf8Name, result);
}

__declspec(dllexport) napi_status My_napi_set_named_property(napi_env env, napi_value object, const char* utf8Name, napi_value value)
{
    spdlog::info("My_napi_set_named_property called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_set_named_property");
    if (!p) { spdlog::error("Original napi_set_named_property not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value,const char*,napi_value);
    return ((Fn)p)(env, object, utf8Name, value);
}

__declspec(dllexport) napi_status My_napi_create_reference(napi_env env, napi_value value, uint32_t initial_refcount, napi_ref* result)
{
    spdlog::info("My_napi_create_reference called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_create_reference");
    if (!p) { spdlog::error("Original napi_create_reference not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value,uint32_t,napi_ref*);
    return ((Fn)p)(env, value, initial_refcount, result);
}

__declspec(dllexport) napi_status My_napi_delete_reference(napi_env env, napi_ref ref)
{
    spdlog::info("My_napi_delete_reference called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_delete_reference");
    if (!p) { spdlog::error("Original napi_delete_reference not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_ref);
    return ((Fn)p)(env, ref);
}

__declspec(dllexport) napi_status My_napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result)
{
    spdlog::info("My_napi_get_reference_value called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_get_reference_value");
    if (!p) { spdlog::error("Original napi_get_reference_value not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_ref,napi_value*);
    return ((Fn)p)(env, ref, result);
}

__declspec(dllexport) napi_status My_napi_typeof(napi_env env, napi_value value, napi_valuetype* result)
{
    spdlog::info("My_napi_typeof called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_typeof");
    if (!p) { spdlog::error("Original napi_typeof not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,napi_value,napi_valuetype*);
    return ((Fn)p)(env, value, result);
}

__declspec(dllexport) napi_status My_napi_is_exception_pending(napi_env env, bool* result)
{
    spdlog::info("My_napi_is_exception_pending called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_is_exception_pending");
    if (!p) { spdlog::error("Original napi_is_exception_pending not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,bool*);
    return ((Fn)p)(env, result);
}

__declspec(dllexport) napi_status My_napi_get_last_error_info(napi_env env, const napi_extended_error_info** result)
{
    spdlog::info("My_napi_get_last_error_info called");
    HMODULE h = get_node(); if (!h) return napi_invalid_arg;
    auto p = (FARPROC)GetProcAddress(h, "napi_get_last_error_info");
    if (!p) { spdlog::error("Original napi_get_last_error_info not found"); return napi_invalid_arg; }
    using Fn = napi_status(*)(napi_env,const napi_extended_error_info**);
    return ((Fn)p)(env, result);
}

// A couple of libuv wrappers
__declspec(dllexport) int My_uv_run(void* loop)
{
    spdlog::info("My_uv_run called");
    HMODULE h = get_node(); if (!h) return -1;
    auto p = (FARPROC)GetProcAddress(h, "uv_run");
    if (!p) { spdlog::error("Original uv_run not found"); return -1; }
    using Fn = int(*)(void*);
    return ((Fn)p)(loop);
}

__declspec(dllexport) int My_uv_timer_start(void* handle, void* cb, uint64_t timeout, uint64_t repeat)
{
    spdlog::info("My_uv_timer_start called");
    HMODULE h = get_node(); if (!h) return -1;
    auto p = (FARPROC)GetProcAddress(h, "uv_timer_start");
    if (!p) { spdlog::error("Original uv_timer_start not found"); return -1; }
    using Fn = int(*)(void*, void(*)(void*), uint64_t, uint64_t);
    // cast cb to appropriate callback type
    return ((Fn)p)(handle, (void(*)(void*))cb, timeout, repeat);
}

// Wrapper for mangled C++ symbol: ?GetCurrent@Isolate@v8@@SAPEAV12@XZ
// Original signature: static Isolate* Isolate::GetCurrent()
extern "C" __declspec(dllexport) void* My_GetCurrent_Isolate_v8()
{
    spdlog::info("My_GetCurrent_Isolate_v8 called");
    HMODULE h = get_orig(); if (!h) return nullptr;
    auto p = (FARPROC)GetProcAddress(h, "?GetCurrent@Isolate@v8@@SAPEAV12@XZ");
    if (!p) { spdlog::error("Original ?GetCurrent@Isolate@v8@@SAPEAV12@XZ not found"); return nullptr; }
    using Fn = void* (*)();
    return ((Fn)p)();
}

// Wrapper for mangled C++ symbol: ?IsEnvironmentStopping@node@@YA_NPEAVIsolate@v8@@@Z
// Original signature: bool IsEnvironmentStopping(node::Isolate*) (approx)
extern "C" __declspec(dllexport) int My_IsEnvironmentStopping_node(void* isolate)
{
    spdlog::info("My_IsEnvironmentStopping_node called");
    HMODULE h = get_orig(); if (!h) return 0;
    auto p = (FARPROC)GetProcAddress(h, "?IsEnvironmentStopping@node@@YA_NPEAVIsolate@v8@@@Z");
    if (!p) { spdlog::error("Original ?IsEnvironmentStopping@node@@YA_NPEAVIsolate@v8@@@Z not found"); return 0; }
    using Fn = bool(*)(void*);
    bool ret = ((Fn)p)(isolate);
    return ret ? 1 : 0;
}

} // extern "C"

