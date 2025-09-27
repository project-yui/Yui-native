#include "include/native_msf.hh"
#include "include/msf_worker.hh"
#include <cstdint>
#include <cstdlib>
#include <stdlib.h>
#include <sys/types.h>
#ifdef __linux__
#include <unistd.h>
#endif
#ifdef _WIN32
#include <process.h>
#endif
#include "include/disasm.hh"
#include "include/install.hh"
#include "spdlog/spdlog.h"
#include <napi.h>
#include <subhook.h>
#include <vector>


static Napi::Object install_hook(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  spdlog::info("check arguments: {}", info.Length());
  if (info.Length() < 2) {
    throw Napi::Error::New(env, "arguments error!");
  }
  spdlog::debug("check arguments ok!");
  if (!info[0].IsString()) {
    throw Napi::Error::New(env, "First argument must be string!");
  }
  if (!info[1].IsObject()) {
    throw Napi::Error::New(env, "Second argument must be object!");
  }
  auto module_name = info[0].As<Napi::String>();
  auto sig_obj = info[1].As<Napi::Object>();
  Napi::Object result = Napi::Object::New(env);
  std::string name = module_name.Utf8Value();

  subhook_set_disasm_handler(custom_disasm);

  spdlog::info("install hook for msf!");
  auto msf = sig_obj.Get("msf");
  if (msf.IsArray()) {
    auto sig = msf.As<Napi::Array>();
    spdlog::debug("signature length: {}", sig.Length());
    std::vector<uint8_t> code;
    for (int i = 0; i < sig.Length(); i++) {
      uint8_t v = sig.Get(i).ToNumber().Int32Value();
      code.emplace_back(v);
    }
    bool ret = install_msf_hook(name, code);
    result.Set("msf", ret);
  }

  spdlog::info("install hook for msf response!");
  auto msfResp = sig_obj.Get("msf_resp");
  if (msfResp.IsArray()) {
    auto sig = msfResp.As<Napi::Array>();
    spdlog::debug("signature length: {}", sig.Length());
    std::vector<uint8_t> code;
    for (int i = 0; i < sig.Length(); i++) {
      uint8_t v = sig.Get(i).ToNumber().Int32Value();
      code.emplace_back(v);
    }
    bool ret = install_msf_response_hook(name, code);
    result.Set("msfResp", ret);
  }
  return result;
}


/**
 * @brief 添加消息体
 *
 * @param info
 * @return Napi::Promise
 */
static Napi::Promise add_pkg(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (info.Length() < 1) {
    throw Napi::Error::New(env, "参数不足!");
  }
  auto arg1 = info[0];
  if (!arg1.IsObject()) {
    throw Napi::Error::New(env, "参数需要是对象类型!");
  }
  auto pkgInfo = arg1.As<Napi::Object>();

  auto uin = pkgInfo.Get("uin");
  if (!uin.IsString() && !uin.IsNumber()) {
    throw Napi::Error::New(env, "uin需要是字符串或数字类型!");
  }

  auto cmd = pkgInfo.Get("cmd");
  if (!cmd.IsString()) {
    throw Napi::Error::New(env, "cmd需要是字符串类型!");
  }

  auto dataN = pkgInfo.Get("data");
  if (!dataN.IsBuffer()) {
    throw Napi::Error::New(env, "data需要是数组类型!");
  }

  yui::CustomTaskPkg pkg;

  pkg.cmd = cmd.As<Napi::String>().Utf8Value();
  if (uin.IsString()) {
    pkg.uin = uin.As<Napi::String>().Utf8Value();
  } else if (uin.IsNumber()) {
    pkg.uin = std::to_string(uin.As<Napi::Number>().Int64Value());
  }
  auto data = dataN.As<Napi::Uint8Array>();
  for (int i = 0; i < data.ByteLength(); i++) {
    uint8_t v = data.Get(i).ToNumber().Int32Value();
    pkg.data.emplace_back(v);
  }

  auto deferred = Napi::Promise::Deferred::New(env);
  MsfWorker *piWorker = new MsfWorker(env, pkg, deferred);
  spdlog::debug("queue...");
  piWorker->Queue();
  spdlog::debug("queue done.");
  return deferred.Promise();
}
static Napi::Object Init(Napi::Env env, Napi::Object exports) {

#ifdef NATIVE_DEBUG
  // #warning "NATIVE_DEBUG is enabled!"
  spdlog::set_level(spdlog::level::debug);
#endif

  spdlog::info("module init!");
  // freopen_s(reinterpret_cast<FILE **> stdout, "CONOUT$", "w", stdout);

  spdlog::debug("init method of module...");
  exports.Set(Napi::String::New(env, "install"),
              Napi::Function::New(env, install_hook));
  exports.Set(Napi::String::New(env, "addPkg"),
              Napi::Function::New(env, add_pkg));

  return exports;
}

NODE_API_MODULE(cmnative, Init)