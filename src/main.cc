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
    std::cout << "Hello, world!\n";
    
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
    printf("current pid:%d\n", p);
    yukihana::hook.reset(new NTNative::LinuxHook(p, target));

    printf("set_signature\n");
    yukihana::hook->set_signature(feature_code);

    printf("install\n");
    yukihana::hook->install((void *)yukihana::execute);
}

static Napi::Boolean install(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::cout << "check arguments: " << info.Length() << std::endl;
  if (info.Length() < 1) {
    throw Napi::Error::New(env, "arguments error!");
  }
  std::cout << "check arguments ok!" << std::endl;
  if (!info[0].IsArray()) {
    return Napi::Boolean::New(env, false);
  }
  auto sig = info[0].As<Napi::Array>();
  std::cout << "length:" << sig.Length() << std::endl;
  std::vector<uint8_t> code;
  for (int i=0; i < sig.Length(); i++) {
    uint8_t v = sig.Get(i).ToNumber().Int32Value();
    code.emplace_back(v);
  }
  install_hook(code);
  return Napi::Boolean::New(env, true);
}

static Napi::Boolean test2(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto failed = Napi::Boolean::New(env, false);
  // sqlite测试
  std::cout << "test sqlite3" << std::endl;
  int rc = 0;
  rc = sqlite3_initialize(); // Initializes the library. If the library has already been initialized it has no effect.
  if (rc != SQLITE_OK)
  {
    std::cout << "failed to init sqlite3!" << std::endl;
    return failed;
  }
  
  sqlite3 *db = nullptr;
  const char *vfs = nullptr;
  rc = sqlite3_open_v2("./test/test.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, vfs);
  if (rc != SQLITE_OK)
  {
    sqlite3_close(db);
    std::cout << "failed to open db!" << std::endl;
    return failed;
  }

  // Now we create an SQL command which is stored in an sqlite3_stmt data structure.
  // Note symColName_ is a member of EquityDataLocator
  sqlite3_stmt *stmt = nullptr;
  std::string s = "SELECT * FROM group_msg_table WHERE \"40027\" = 933286835;";
  rc = sqlite3_prepare_v2(db, s.c_str(), s.size() + 1, &stmt, nullptr);
  if (rc != SQLITE_OK)
  {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::cout << "failed to prepare sql!" << std::endl;
    return failed;
  }

  
  auto querySql = sqlite3_sql(stmt);
  std::cout << "sql:" << querySql << std::endl;
  // Vdbe * info = (Vdbe *)stmt;
  
  // Now we retrieve the row
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW)
  {
    // Here we get a pointer to the location text ( stored in the second column of the table )
    // The 1 in sqlite3_column_text( stmt, 1 ) is the column number (zero based).
    // sqlite3_column_text( sqlite_stmt* stmt, int cidx ) returns const unsigned char* so the casts are necessary.
    void *p = const_cast<unsigned char *>(sqlite3_column_text(stmt, 1));
    const char *ret = static_cast<const char *>(p);
    std::cout << "data:" << ret << std::endl;
  }
  else
  {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return failed;
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
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
  std::cout << "msgId" << std::endl;
  auto msgId = data.Get("msgId").As<Napi::String>();
  m.msgId = atol(msgId.Utf8Value().c_str());
  printf("msgId: %ld\n", m.msgId);
  std::cout << "msgRandom" << std::endl;
  auto msgRandom = data.Get("msgRandom").As<Napi::Number>();
  m.msgRandom = msgRandom.Int64Value();
  std::cout << "msgSeq" << std::endl;
  auto msgSeq = data.Get("msgSeq").As<Napi::Number>();
  m.msgSeq = msgSeq.Int32Value();
  std::cout << "chatType" << std::endl;
  auto chatType = data.Get("chatType").As<Napi::Number>();
  m.chatType = chatType.Int32Value();
  std::cout << "msgType" << std::endl;
  auto msgType = data.Get("msgType").As<Napi::Number>();
  m.msgType = msgType.Int32Value();
  auto subMsgType = data.Get("subMsgType").As<Napi::Number>();
  m.subMsgType = subMsgType.Int32Value();
  auto sendType = data.Get("sendType").As<Napi::Number>();
  m.sendType = sendType.Int32Value();
  std::cout << "senderUid" << std::endl;
  auto senderUid = data.Get("senderUid").As<Napi::String>();
  m.senderUid = senderUid.Utf8Value();
  printf("senderUid: %s\n", m.senderUid.c_str());
  auto peerUid = data.Get("peerUid").As<Napi::String>();
  m.peerUid = peerUid.Utf8Value();
  m.peerUidLong = atol(m.peerUid.c_str());
  std::cout << "msgTime" << std::endl;
  auto msgTime = data.Get("msgTime").As<Napi::Number>();
  m.msgTime = msgTime.Int64Value();
  std::cout << "sendStatus" << std::endl;
  auto sendStatus = data.Get("sendStatus").As<Napi::Number>();
  m.sendStatus = sendStatus.Int32Value();
  std::cout << "sendMemberName" << std::endl;
  auto sendMemberName = data.Get("sendMemberName").As<Napi::String>();
  m.sendMemberName.reset(new std::string(sendMemberName.Utf8Value()));
  std::cout << "sendNickName" << std::endl;
  auto sendNickName = data.Get("sendNickName").As<Napi::String>();
  m.sendNickName.reset(new std::string(sendNickName.Utf8Value()));
  // elements
  auto elements = data.Get("elements").As<Napi::Array>();
  std::cout << "elements" << std::endl;
  convertNapi2Buf(elements, m.elements);
  std::cout << "senderUin" << std::endl;
  auto senderUin = data.Get("senderUin").As<Napi::Number>();
  m.senderUin = senderUin.Int64Value();
  std::cout << "clientSeq" << std::endl;
  auto clientSeq = data.Get("clientSeq").As<Napi::Number>();
  m.clientSeq = clientSeq.Int32Value();
  std::cout << "atType" << std::endl;
  auto atType = data.Get("atType").As<Napi::Number>();
  m.atType = atType.Int32Value();
  
  nt_db::GroupMsgTableDb db;
  
  std::cout << "db add start" << std::endl;
  db.add(m);
  return Napi::Boolean::New(env, true);
}
static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  printf("module init\n");
  // https://github.com/chrononeko/bugtracker/issues/7
  // freopen_s(reinterpret_cast<FILE **> stdout, "CONOUT$", "w", stdout);

  exports.Set(
      Napi::String::New(env, "install"),
      Napi::Function::New(env, install));
  // exports.Set(
  //     Napi::String::New(env, "test2"),
  //     Napi::Function::New(env, test2));
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