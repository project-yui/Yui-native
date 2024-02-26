#include "../include/hook.hh"

namespace NTNative {

bool Hook::set_signature(const std::vector<uint8_t> &_signature) {
  if (hook.IsInstalled())
    return false;
  signature = _signature;
  return true;
};

bool Hook::install(void *dest) {
  auto func = get_start_addr();
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
    // 找到地址
    auto ptr = search_feature_code((const uint8_t *)addrRange.first,
                                   addrRange.second - addrRange.first);
    return (void *)ptr;
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