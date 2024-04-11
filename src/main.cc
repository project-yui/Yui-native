#include "include/db/group_msg_table.hh"
#include "include/nt/element.hh"
#include "include/handle.hh"
#include "proto/message.pb.h"
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
#include "include/windows_hook.hh"
#endif
#include <vector>
#include <napi.h>
#include <subhook.h>
#include <sqlite3.h>
#include "test/test_sqlite3.hh"
#include "spdlog/spdlog.h"
#include "include/convert.hh"

/**
 * @brief 安装hook
 * 
 * @param feature_code 
 */
static bool install_sqlite3_hook(std::vector<uint8_t> & feature_code) {
    spdlog::info("internal install hook");
    
    std::string target = "wrapper.node";
    // init();
    sqlite3_initialize();
    // printf("pid of this process:%d\n", pid);
    // getNameByPid(pid, task_name);

    /*
    strcpy(task_name, argv[0]+2);
    printf("task name is %s\n", task_name);
    getPidByName(task_name);
    */
#ifdef __linux__
    pid_t p = getpid();
#endif
#ifdef _WIN32
    pid_t p = _getpid();
#endif
    spdlog::debug("current pid: {}\n", p);
    #ifdef __linux__
    yukihana::sqlit3_stmt_hooker.reset(new NTNative::LinuxHook(p, target));
    #endif
    #ifdef _WIN32
    yukihana::sqlit3_stmt_hooker.reset(new NTNative::WindowsHook(p, target));
    #endif

    spdlog::debug("set_signature\n");
    yukihana::sqlit3_stmt_hooker->set_signature(feature_code);

    spdlog::debug("install\n");
    return yukihana::sqlit3_stmt_hooker->install((void *)yukihana::sqlite3_stmt_hook);
}
static bool install_hosts_hook(std::vector<uint8_t> & feature_code) {
    spdlog::info("internal install hook");
    
    std::string target = "wrapper.node";
#ifdef __linux__
    pid_t p = getpid();
#endif
#ifdef _WIN32
    pid_t p = _getpid();
#endif
    spdlog::debug("current pid: {}\n", p);
    #ifdef __linux__
    yukihana::hosts_hooker.reset(new NTNative::LinuxHook(p, target));
    #endif
    #ifdef _WIN32
    yukihana::hosts_hooker.reset(new NTNative::WindowsHook(p, target));
    #endif

    spdlog::debug("set_signature\n");
    yukihana::hosts_hooker->set_signature(feature_code);

    spdlog::debug("install\n");
    return yukihana::hosts_hooker->install((void *)yukihana::hosts_hook);
}

static Napi::Object install_hook(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  spdlog::info("check arguments: {}", info.Length());
  if (info.Length() < 1) {
    throw Napi::Error::New(env, "arguments error!");
  }
  spdlog::debug("check arguments ok!");
  if (!info[0].IsObject()) {
    throw Napi::Error::New(env, "First argument must be object!");
  }
  auto sig_obj = info[0].As<Napi::Object>();
  Napi::Object result = Napi::Object::New(env);

  auto sqlite3_stmt = sig_obj.Get("sqlite3_stmt");
  if (sqlite3_stmt.IsArray())
  {
    auto sig = sqlite3_stmt.As<Napi::Array>();
    spdlog::debug("signature length: {}", sig.Length());
    std::vector<uint8_t> code;
    for (int i=0; i < sig.Length(); i++) {
      uint8_t v = sig.Get(i).ToNumber().Int32Value();
      code.emplace_back(v);
    }
    bool ret = install_sqlite3_hook(code);
    result.Set("sqlite3_stmt", ret);
  }
  
  auto hosts = sig_obj.Get("hosts");
  if (hosts.IsArray())
  {
    auto sig = hosts.As<Napi::Array>();
    spdlog::debug("signature length: {}", sig.Length());
    std::vector<uint8_t> code;
    for (int i=0; i < sig.Length(); i++) {
      uint8_t v = sig.Get(i).ToNumber().Int32Value();
      code.emplace_back(v);
    }
    bool ret = install_hosts_hook(code);
    result.Set("hosts", ret);
  }
  return result;
}

/**
 * @brief 添加group消息
 * 
 * @param info 
 * @return Napi::Boolean 
 */
static Napi::Boolean addMsg(const Napi::CallbackInfo &info) {
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
static Napi::Object Init(Napi::Env env, Napi::Object exports) {

#ifdef NATIVE_DEBUG
  // #warning "NATIVE_DEBUG is enabled!"
  spdlog::set_level(spdlog::level::debug);
#endif

  spdlog::info("module init!");
  // https://github.com/chrononeko/bugtracker/issues/7
  // freopen_s(reinterpret_cast<FILE **> stdout, "CONOUT$", "w", stdout);

  spdlog::debug("init method of module...");
  exports.Set(
      Napi::String::New(env, "install"),
      Napi::Function::New(env, install_hook));
  exports.Set(
      Napi::String::New(env, "addMsg"),
      Napi::Function::New(env, addMsg));

//   exports.Set(
//       Napi::String::New(env, "setPBPreprocessorForGzHook"),
//       Napi::Function::New(env, setPBPreprocessorForGzHook));

//   exports.Set(
//       Napi::String::New(env, "setHandlerForPokeHook"),
//       Napi::Function::New(env, setHandlerForPokeHook));

  return exports;
}

NODE_API_MODULE(cmnative, Init)