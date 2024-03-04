#include "../include/windows_hook.hh"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <psapi.h>

namespace NTNative {

  HMODULE GetProcessModuleHandle(DWORD pid, const char* moduleName){	// 根据 PID 、模块名（需要写后缀，如：".dll"），获取模块入口地址。
      MODULEENTRY32 moduleEntry;
      HANDLE handle = NULL;
      handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid); //  获取进程快照中包含在th32ProcessID中指定的进程的所有的模块。
      if (!handle) {
          CloseHandle(handle); 
          return NULL;
      }
      ZeroMemory(&moduleEntry, sizeof(MODULEENTRY32));
      moduleEntry.dwSize = sizeof(MODULEENTRY32);
      if (!Module32First(handle, &moduleEntry)) {
          CloseHandle(handle); 
          return NULL;
      }
      do {
          if (_tcscmp(moduleEntry.szModule, moduleName) == 0) {return moduleEntry.hModule;}
      } while (Module32Next(handle, &moduleEntry));
      CloseHandle(handle); 
      return 0;
  }
  std::pair<unsigned long, unsigned long> WindowsHook::get_module_address() {
      auto moduleHandle = GetProcessModuleHandle(pid, m_moduleName.c_str());
      MODULEINFO moduleInfo;
      typedef BOOL(WINAPI* LPFN_GetModuleInformation)(
          HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

      HMODULE hPsapi = LoadLibraryW(L"PSAPI.DLL");
      LPFN_GetModuleInformation pGetModuleInformation = NULL;
      pGetModuleInformation =
          (LPFN_GetModuleInformation)GetProcAddress(hPsapi, "GetModuleInformation");
      pGetModuleInformation(
          GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(MODULEINFO));

    std::pair<unsigned long, unsigned long> r((unsigned long)moduleInfo.lpBaseOfDll, (unsigned long)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage);
    return r;
  }
} // namespace NTNative

#endif