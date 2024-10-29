#include "../include/hook.hh"
#include "subhook.h"
#include <cstdint>
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
    for (size_t i = 0; i < size * 0.9; ++i) {
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
} // namespace NTNative