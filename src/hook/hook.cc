#include "../include/hook.hh"
#include "subhook.h"
#include <cstdint>
#include <cstring>
#include <spdlog/spdlog.h>
namespace NTNative {

bool Hook::set_signature(const std::vector<uint8_t> &_signature) {
  if (hook.IsInstalled())
    return false;
  signature = _signature;
  return true;
};

bool Hook::install(void *dest) {
    spdlog::debug("start install hook");
  auto func = get_start_addr();
  spdlog::debug("start address: {}", func);
  if (func == nullptr)
    return false;
  if (hook.IsInstalled())
    return false;
  original_func = func;
  // nearby: 如果分配的内存过远会被subhook判定溢出
  bool ret =  hook.Install(func, dest, subhook::HookFlag64BitOffset | subhook::HookFlagTrampoline | subhook::HookFlagTrampolineAllocNearby);
  
  spdlog::debug("trampoline start");
  void * trampoline = hook.GetTrampoline();
  spdlog::debug("trampoline: {}", trampoline);
  if (trampoline != nullptr)
  {
    spdlog::debug("hex: {:#04X} {:#04X} {:#04X} {:#04X} {:#04X} {:#04X} {:#04X} {:#04X}", ((uint8_t *)trampoline)[0], ((uint8_t *)trampoline)[1], ((uint8_t *)trampoline)[2], ((uint8_t *)trampoline)[3], ((uint8_t *)trampoline)[4], ((uint8_t *)trampoline)[5], ((uint8_t *)trampoline)[6], ((uint8_t *)trampoline)[7]);
  }
  return ret;
}

void *Hook::get_start_addr() {
  auto addrRange = get_module_address();
  if (addrRange.first != 0) {
      spdlog::debug("module address range: {} -> {}(size)", addrRange.first, addrRange.second);
      spdlog::debug("module address first: {}", *(char *)addrRange.first);
    // 找到地址
    auto ptr = search_feature_code((const uint8_t *)addrRange.first,
                                   addrRange.second);
    spdlog::debug("search signature result: {}", (void *)ptr);
    return (void *)ptr;
  }
  else {
      spdlog::warn("module address cal error!!");
  }
  return nullptr;
}

bool Hook::is_feature_code_matched(const uint8_t *data) {
    for (size_t i = 0; i < signature.size(); ++i) {
        if (data[i] != signature[i]) {
            return false;
        }
    }
    return true;
}

const uint8_t *Hook::search_feature_code(const uint8_t *data, size_t size) {
    spdlog::debug("search_feature_code: {} - {}", (void *)data, size);
    const uint8_t * result = nullptr;
    for (size_t i = 0; i < size - signature.size(); ++i) {
        // spdlog::info("Check: {}", i);
        if (is_feature_code_matched(data + i)) {
            if (result != nullptr)
            {
                spdlog::error("More than one position was found!");
                return nullptr;
            }
            result = data + i;
        }
    }
    return result;
}

void * Hook::get_trampoline() {
  return hook.GetTrampoline();
}
void * Hook::auto_search_func_address(std::string& keyword) {
  // 1. 搜索字符串，得到“字符串位置”
  auto addrRange = get_module_address();
  uint8_t * strAddr = nullptr;
  for (long start = addrRange.second; start > 0; start--) {
    auto addr = (char*)addrRange.first + start;
    if (addr == strstr(addr, keyword.data()))
    {
      while (*addr != 0) {
        strAddr = (uint8_t *)addr;
        addr--;
      }
      break;
    }
  }

  // 2. 往前搜索表达式：指令位置 + 指令长度 + 指令立即数 = “字符串位置”
  // 下一个指令位置 + 32位指令立即数 = “字符串位置”
  uint8_t * refAddr = nullptr;
  for (uint8_t* nextCmd = strAddr - 1; nextCmd > (uint8_t *)addrRange.first + 7; nextCmd--) {
    // 48 8D 05 88 D8 27 03
    auto prefix = *(nextCmd - 7);
    auto opCode = *(nextCmd - 6);
    auto mod = *(nextCmd - 5);
    if (prefix != 0x48 || opCode != 0x8D || mod != 0x05) continue;

    long offset = *(nextCmd - 1) < 8*3 + *(nextCmd - 2) < 8*2+ *(nextCmd - 3) < 8*1 + *(nextCmd - 4);
    if (nextCmd + offset == strAddr)
    {
      refAddr = nextCmd - 7;
      break;
    }
  }

  // 3. 往 "下一个指令位置" 前搜索函数起始地址
  if (keyword == "stmt")
  {
    uint8_t signature[] = {
      0x41, 0x56,
      0x56,
      0x57,
      0x55,
      0x53
    };
    int size = 6;
    for (auto addr = refAddr - size; addr > (uint8_t *)addrRange.first; addr--) {
      bool result = true;
      for (int i=0; i < size; i++ ) {
        if(addr[i] != signature[i])
        {
          result = false;
        }
      }
      if (result)
      {
        return (void *)addr;
      }
    }
  }

  return nullptr;
}

} // namespace NTNative