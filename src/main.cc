#include "include/db/group_msg_table.hh"
#include "include/native_msf.hh"
#include "include/msf_worker.hh"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdlib.h>
#include <sys/types.h>
#ifdef __linux__
#include "include/linux_hook.hh"
#include <unistd.h>
#endif
#ifdef _WIN32
#include <process.h>
#endif
#include "include/convert.hh"
#include "include/disasm.hh"
#include "include/install.hh"
#include "spdlog/spdlog.h"
#include <napi.h>
#include <sqlite3.h>
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

  spdlog::info("install hook for sqlite3_stmt!");
  auto sqlite3_stmt = sig_obj.Get("sqlite3_stmt");
  if (sqlite3_stmt.IsArray()) {
    auto sig = sqlite3_stmt.As<Napi::Array>();
    spdlog::debug("signature length: {}", sig.Length());
    std::vector<uint8_t> code;
    for (int i = 0; i < sig.Length(); i++) {
      uint8_t v = sig.Get(i).ToNumber().Int32Value();
      code.emplace_back(v);
    }
    bool ret = install_sqlite3_hook(name, code);
    result.Set("sqlite3_stmt", ret);
  }

  spdlog::info("install hook for hosts!");
  auto hosts = sig_obj.Get("hosts");
  if (hosts.IsArray()) {
    auto sig = hosts.As<Napi::Array>();
    spdlog::debug("signature length: {}", sig.Length());
    std::vector<uint8_t> code;
    for (int i = 0; i < sig.Length(); i++) {
      uint8_t v = sig.Get(i).ToNumber().Int32Value();
      code.emplace_back(v);
    }
    bool ret = install_hosts_hook(name, code);
    result.Set("hosts", ret);
  }

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
 * @brief 添加group消息
 *
 * @param info
 * @return Napi::Boolean
 */
static Napi::Boolean add_msg(const Napi::CallbackInfo &info) {
  spdlog::debug("Call addMsg...");
  auto env = info.Env();
  if (info.Length() == 0) {
    throw Napi::Error::New(env, "参数长度不能为0!");
  }
  auto msg = info[0];
  if (!msg.IsObject()) {
    throw Napi::Error::New(env, "参数必须是对象!");
  }
  auto data = msg.As<Napi::Object>();

  // Object 转 struct
  nt_model::GroupMsgTable m;

  spdlog::debug("get msgId...");
  auto msgId = data.Get("msgId").As<Napi::String>();
  m.msgId = atoll(msgId.Utf8Value().c_str());

  spdlog::debug("get msgRandom...");
  auto msgRandom = data.Get("msgRandom").As<Napi::Number>();
  m.msgRandom = msgRandom.Int64Value();

  spdlog::debug("get msgSeq...");
  auto msgSeq = data.Get("msgSeq").As<Napi::Number>();
  m.msgSeq = msgSeq.Int32Value();

  spdlog::debug("get chatType...");
  auto chatType = data.Get("chatType").As<Napi::Number>();
  m.chatType = chatType.Int32Value();

  spdlog::debug("get msgType...");
  auto msgType = data.Get("msgType").As<Napi::Number>();
  m.msgType = msgType.Int32Value();

  spdlog::debug("get subMsgType...");
  auto subMsgType = data.Get("subMsgType").As<Napi::Number>();
  m.subMsgType = subMsgType.Int32Value();

  spdlog::debug("get sendType...");
  auto sendType = data.Get("sendType").As<Napi::Number>();
  m.sendType = sendType.Int32Value();

  spdlog::debug("get senderUid...");
  auto senderUid = data.Get("senderUid").As<Napi::String>();
  m.senderUid = senderUid.Utf8Value();

  spdlog::debug("get peerUid...");
  auto peerUid = data.Get("peerUid").As<Napi::String>();
  m.peerUid = peerUid.Utf8Value();

  spdlog::debug("get msgTime...");
  m.peerUidLong = atol(m.peerUid.c_str());
  auto msgTime = data.Get("msgTime").As<Napi::Number>();
  m.msgTime = msgTime.Int64Value();

  spdlog::debug("get sendStatus...");
  auto sendStatus = data.Get("sendStatus").As<Napi::Number>();
  m.sendStatus = sendStatus.Int32Value();

  spdlog::debug("get sendMemberName...");
  auto sendMemberName = data.Get("sendMemberName").As<Napi::String>();
  m.sendMemberName.reset(new std::string(sendMemberName.Utf8Value()));

  spdlog::debug("get sendNickName...");
  auto sendNickName = data.Get("sendNickName").As<Napi::String>();
  m.sendNickName.reset(new std::string(sendNickName.Utf8Value()));

  spdlog::debug("get elements...");
  // elements
  auto elements = data.Get("elements").As<Napi::Array>();
  // convertNapi2Buf(elements, m.elements);
  nt_convert::ElementConverter::getInstance().toProtobuf(elements, m.elements);

  spdlog::debug("get senderUin...");
  auto senderUin = data.Get("senderUin").As<Napi::Number>();
  m.senderUin = senderUin.Int64Value();

  spdlog::debug("get clientSeq...");
  auto clientSeq = data.Get("clientSeq").As<Napi::Number>();
  m.clientSeq = clientSeq.Int32Value();

  spdlog::debug("get atType...");
  auto atType = data.Get("atType").As<Napi::Number>();
  m.atType = atType.Int32Value();
  m.todayZero = m.msgTime - m.msgTime % (24 * 60 * 60) - 8 * 60 * 60;
  m.userLevel = 3;

  spdlog::debug("Create db handle...");
  nt_db::GroupMsgTableDb db;

  auto result = db.add(m);
  return Napi::Boolean::New(env, result);
}

/**
 * @brief 添加group消息
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
  exports.Set(Napi::String::New(env, "addMsg"),
              Napi::Function::New(env, add_msg));
  exports.Set(Napi::String::New(env, "addPkg"),
              Napi::Function::New(env, add_pkg));

  return exports;
}

NODE_API_MODULE(cmnative, Init)