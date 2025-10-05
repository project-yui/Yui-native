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
extern "C"{  
    __declspec(dllexport) int My_IsEnvironmentStopping_node(void* isolate)
    {
        spdlog::info("My_IsEnvironmentStopping_node called");
        return 0;
    }
    
    __declspec(dllexport) void My_QQMain()
    {
        spdlog::info("My_QQMain called");
        // You can add additional logic here if needed
    }
    
    __declspec(dllexport) void My_PerfTrace()
    {
        spdlog::info("My_PerfTrace called");
        // You can add additional logic here if needed
    }

    __declspec(dllexport) void My_sqlite3_dbdata_init()
    {
        spdlog::info("My_sqlite3_dbdata_init called");
        // You can add additional logic here if needed
    }
    
    __declspec(dllexport) void My_Cr_z_adler32()
    {
        spdlog::info("My_Cr_z_adler32 called");
    }

    __declspec(dllexport) void My_Cr_z_adler32_combine()
    {
        spdlog::info("My_Cr_z_adler32_combine called");
    }

    __declspec(dllexport) void My_Cr_z_adler32_z()
    {
        spdlog::info("My_Cr_z_adler32_z called");
    }

    __declspec(dllexport) void My_Cr_z_compress()
    {
        spdlog::info("My_Cr_z_compress called");
    }

    __declspec(dllexport) void My_Cr_z_compress2()
    {
        spdlog::info("My_Cr_z_compress2 called");
    }

    __declspec(dllexport) void My_Cr_z_compressBound()
    {
        spdlog::info("My_Cr_z_compressBound called");
    }

    __declspec(dllexport) void My_Cr_z_crc32()
    {
        spdlog::info("My_Cr_z_crc32 called");
    }

    __declspec(dllexport) void My_Cr_z_crc32_combine()
    {
        spdlog::info("My_Cr_z_crc32_combine called");
    }

    __declspec(dllexport) void My_Cr_z_crc32_combine_gen()
    {
        spdlog::info("My_Cr_z_crc32_combine_gen called");
    }

    __declspec(dllexport) void My_Cr_z_crc32_combine_op()
    {
        spdlog::info("My_Cr_z_crc32_combine_op called");
    }

    __declspec(dllexport) void My_Cr_z_crc32_z()
    {
        spdlog::info("My_Cr_z_crc32_z called");
    }

    __declspec(dllexport) void My_Cr_z_deflate()
    {
        spdlog::info("My_Cr_z_deflate called");
    }

    __declspec(dllexport) void My_Cr_z_deflateBound()
    {
        spdlog::info("My_Cr_z_deflateBound called");
    }

    __declspec(dllexport) void My_Cr_z_deflateCopy()
    {
        spdlog::info("My_Cr_z_deflateCopy called");
    }

    __declspec(dllexport) void My_Cr_z_deflateEnd()
    {
        spdlog::info("My_Cr_z_deflateEnd called");
    }

    __declspec(dllexport) void My_Cr_z_deflateGetDictionary()
    {
        spdlog::info("My_Cr_z_deflateGetDictionary called");
    }

    __declspec(dllexport) void My_Cr_z_deflateInit2_()
    {
        spdlog::info("My_Cr_z_deflateInit2_ called");
    }

    __declspec(dllexport) void My_Cr_z_deflateInit_()
    {
        spdlog::info("My_Cr_z_deflateInit_ called");
    }

    __declspec(dllexport) void My_Cr_z_deflateParams()
    {
        spdlog::info("My_Cr_z_deflateParams called");
    }

    __declspec(dllexport) void My_Cr_z_deflatePending()
    {
        spdlog::info("My_Cr_z_deflatePending called");
    }

    __declspec(dllexport) void My_Cr_z_deflatePrime()
    {
        spdlog::info("My_Cr_z_deflatePrime called");
    }

    __declspec(dllexport) void My_Cr_z_deflateReset()
    {
        spdlog::info("My_Cr_z_deflateReset called");
    }

    __declspec(dllexport) void My_Cr_z_deflateResetKeep()
    {
        spdlog::info("My_Cr_z_deflateResetKeep called");
    }

    __declspec(dllexport) void My_Cr_z_deflateSetDictionary()
    {
        spdlog::info("My_Cr_z_deflateSetDictionary called");
    }

    __declspec(dllexport) void My_Cr_z_deflateSetHeader()
    {
        spdlog::info("My_Cr_z_deflateSetHeader called");
    }

    __declspec(dllexport) void My_Cr_z_deflateTune()
    {
        spdlog::info("My_Cr_z_deflateTune called");
    }

    __declspec(dllexport) void My_Cr_z_get_crc_table()
    {
        spdlog::info("My_Cr_z_get_crc_table called");
    }

    __declspec(dllexport) void My_Cr_z_uncompress()
    {
        spdlog::info("My_Cr_z_uncompress called");
    }

    __declspec(dllexport) void My_Cr_z_uncompress2()
    {
        spdlog::info("My_Cr_z_uncompress2 called");
    }

    __declspec(dllexport) void My_Cr_z_zError()
    {
        spdlog::info("My_Cr_z_zError called");
    }

    __declspec(dllexport) void My_Cr_z_zlibCompileFlags()
    {
        spdlog::info("My_Cr_z_zlibCompileFlags called");
    }

    __declspec(dllexport) void My_Cr_z_zlibVersion()
    {
        spdlog::info("My_Cr_z_zlibVersion called");
    }

    __declspec(dllexport) void My_ExportedContentMain()
    {
        spdlog::info("My_ExportedContentMain called");
    }

    __declspec(dllexport) void My_GetHandleVerifier()
    {
        spdlog::info("My_GetHandleVerifier called");
    }

    __declspec(dllexport) void My_IsSandboxedProcess()
    {
        spdlog::info("My_IsSandboxedProcess called");
    }

    __declspec(dllexport) void My_NodeContextifyContextMetrics1()
    {
        spdlog::info("My_NodeContextifyContextMetrics1 called");
    }
}
