
#include "napi.h"
#include "spdlog/spdlog.h"
#include <dlfcn.h>
#include <gnutls/gnutls.h>

#if defined(__linux__) || defined(__APPLE__)
extern "C" {
// Keep the existing registration helper
void qq_magic_napi_register(napi_module *m) {
    spdlog::info("call qq_magic_napi_register");
    napi_module_register(m);
}
// Ensure gnutls_xxx is linked in
void * _force_gnutls_global_init __attribute__((used)) = (void *)gnutls_global_init;
}

#endif


static Napi::Object Init(Napi::Env env, Napi::Object exports) {

  return exports;
}

NODE_API_MODULE(cmnative, Init)