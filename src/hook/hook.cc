#include "../include/hook.hh"
#include "subhook.h"
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
  return hook.Install(
      func, dest, subhook::HookFlag64BitOffset | subhook::HookFlagTrampoline);
}

void *Hook::get_start_addr() {
  auto addrRange = get_module_address();
  if (addrRange.first != 0) {
      spdlog::debug("module address range: {} -> {}", addrRange.first, addrRange.second);
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
    for (size_t i = 0; i < size - signature.size(); ++i) {
        if (is_feature_code_matched(data + i)) {
            return data + i;
        }
    }
    return nullptr;
}

void * Hook::get_trampoline() {
  return hook.GetTrampoline();
}
} // namespace NTNative