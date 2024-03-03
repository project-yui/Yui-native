#include "include/db/group_msg_table.hh"
#include "include/linux_hook.hh"
#include "include/nt/element.hh"
#include "include/nt/message.hh"
#include "include/sqlite3/nt/vdbe.hh"
#include "include/sqlite3/nt/sqlite3.hh"
#include "include/sqlite3/nt/base.hh"
#include "include/handle.hh"
#include "proto/message.pb.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sstream>
#include <utility>
#include <vector>
#include <napi.h>
#include <subhook.h>
#include <sqlite3.h>
#include "test/test_sqlite3.hh"
#include "spdlog/spdlog.h"

static void convertELements2buf(std::vector<Element>& elems, std::string &output) {
  nt_msg::Elements elements;
  for (auto elem : elems) {
    auto e = elements.add_elem();
    e->set_elementid(elem.elementId);
    e->set_elementtype(elem.elementType);
    e->set_textstr(elem.textElement.content);
    e->set_attype(elem.textElement.atType);
  }
  elements.SerializeToString(&output);
}

/**
 * @brief 安装hook
 * 
 * @param feature_code 
 */
static void install_hook(std::vector<uint8_t> & feature_code) {
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
    pid_t p = getpid();
    spdlog::debug("current pid:%d\n", p);
    #ifdef __linux__
    yukihana::hook.reset(new NTNative::LinuxHook(p, target));
    #endif
    #ifdef _WIN32
    yukihana::hook.reset(new NTNative::WindowsHook(p, target));
    #endif

    spdlog::debug("set_signature\n");
    yukihana::hook->set_signature(feature_code);

    spdlog::debug("install\n");
    yukihana::hook->install((void *)yukihana::execute);
}

static Napi::Boolean install(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  spdlog::info("check arguments: %d", info.Length());
  if (info.Length() < 1) {
    throw Napi::Error::New(env, "arguments error!");
  }
  spdlog::debug("check arguments ok!");
  if (!info[0].IsArray()) {
    return Napi::Boolean::New(env, false);
  }
  auto sig = info[0].As<Napi::Array>();
  spdlog::debug("length: %d", sig.Length());
  std::vector<uint8_t> code;
  for (int i=0; i < sig.Length(); i++) {
    uint8_t v = sig.Get(i).ToNumber().Int32Value();
    code.emplace_back(v);
  }
  install_hook(code);
  return Napi::Boolean::New(env, true);
}

/**
 * @brief 
 * TODO: 多类型支持
 * 
 * @param data 
 * @param output 
 */
static void convertNapi2Buf(Napi::Array &data, std::vector<char>& output) {
  int cnt = data.Length();
  for (int i=0; i < cnt; i++) {
    auto element = data.Get(i).As<Napi::Object>();
    
    nt_msg::Elements elems;
    auto elem = elems.add_elem();
    elem->set_elementtype(nt_msg::Element_MsgType_MSG_TYPE_TEXT);
    
    auto elementId = element.Get("elementId").As<Napi::String>();
    elem->set_elementid(atol(elementId.Utf8Value().c_str()));

    auto textElement = element.Get("textElement").As<Napi::Object>();
    auto textStr = textElement.Get("content").As<Napi::String>();
    elem->set_textstr(textStr.Utf8Value());
    elem->set_attype(0);
    std::string out;
    elems.SerializeToString(&out);
    for (int j=0; j < out.length(); j++) {
      
      output.push_back(out[j]);
    }
  }
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
  auto msgId = data.Get("msgId").As<Napi::String>();
  m.msgId = atol(msgId.Utf8Value().c_str());
  auto msgRandom = data.Get("msgRandom").As<Napi::Number>();
  m.msgRandom = msgRandom.Int64Value();
  auto msgSeq = data.Get("msgSeq").As<Napi::Number>();
  m.msgSeq = msgSeq.Int32Value();
  auto chatType = data.Get("chatType").As<Napi::Number>();
  m.chatType = chatType.Int32Value();
  auto msgType = data.Get("msgType").As<Napi::Number>();
  m.msgType = msgType.Int32Value();
  auto subMsgType = data.Get("subMsgType").As<Napi::Number>();
  m.subMsgType = subMsgType.Int32Value();
  auto sendType = data.Get("sendType").As<Napi::Number>();
  m.sendType = sendType.Int32Value();
  auto senderUid = data.Get("senderUid").As<Napi::String>();
  m.senderUid = senderUid.Utf8Value();
  auto peerUid = data.Get("peerUid").As<Napi::String>();
  m.peerUid = peerUid.Utf8Value();
  m.peerUidLong = atol(m.peerUid.c_str());
  auto msgTime = data.Get("msgTime").As<Napi::Number>();
  m.msgTime = msgTime.Int64Value();
  auto sendStatus = data.Get("sendStatus").As<Napi::Number>();
  m.sendStatus = sendStatus.Int32Value();
  auto sendMemberName = data.Get("sendMemberName").As<Napi::String>();
  m.sendMemberName.reset(new std::string(sendMemberName.Utf8Value()));
  auto sendNickName = data.Get("sendNickName").As<Napi::String>();
  m.sendNickName.reset(new std::string(sendNickName.Utf8Value()));
  // elements
  auto elements = data.Get("elements").As<Napi::Array>();
  convertNapi2Buf(elements, m.elements);
  auto senderUin = data.Get("senderUin").As<Napi::Number>();
  m.senderUin = senderUin.Int64Value();
  auto clientSeq = data.Get("clientSeq").As<Napi::Number>();
  m.clientSeq = clientSeq.Int32Value();
  auto atType = data.Get("atType").As<Napi::Number>();
  m.atType = atType.Int32Value();
  m.todayZero = m.msgTime - m.msgTime % (24 * 60 * 60) - 8 * 60 * 60;
  m.userLevel = 3;

  spdlog::debug("Create db handle...");
  nt_db::GroupMsgTableDb db;
  
  db.add(m);
  return Napi::Boolean::New(env, true);
}
static Napi::Object Init(Napi::Env env, Napi::Object exports) {

#ifdef NATIVE_DEBUG
 spdlog::set_level(spdlog::level::debug);
#endif

  spdlog::info("module init!");
  // https://github.com/chrononeko/bugtracker/issues/7
  // freopen_s(reinterpret_cast<FILE **> stdout, "CONOUT$", "w", stdout);

  spdlog::debug("init method of module...");
  exports.Set(
      Napi::String::New(env, "install"),
      Napi::Function::New(env, install));
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