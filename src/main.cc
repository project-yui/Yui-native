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
 * @brief 计算当前汇编指令的长度（一条的字节数）
 * 
 * @param src 汇编数据
 * @param reloc_op_offset 0?
 * @return int 长度
 */
int custom_disasm(void *src, int *reloc_op_offset) {
  spdlog::debug("custom disasm start");
  spdlog::debug("src: {}, reloc_op_offset: {} -> ", src, (void *)reloc_op_offset, *reloc_op_offset);
  enum flags {
    MODRM      = 1,
    PLUS_R     = 1 << 1,
    REG_OPCODE = 1 << 2,
    IMM8       = 1 << 3,
    IMM16      = 1 << 4,
    IMM32      = 1 << 5,
    RELOC      = 1 << 6
  };

  static uint8_t prefixes[] = {
    0xF0, 0xF2, 0xF3,
    0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65,
    0x66, /* operand size override */
    0x67  /* address size override */
  };

  struct opcode_info {
    uint8_t opcode;
    uint8_t reg_opcode;
    unsigned int flags;
  };

  /*
   * See the Intel Developer Manual volumes 2a and 2b for more information
   * about instruction format and encoding:
   *
   * https://www-ssl.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html
   */
  static struct opcode_info opcodes[] = {
    /* ADD AL, imm8      */ {0x04, 0, IMM8},
    /* ADD EAX, imm32    */ {0x05, 0, IMM32},
    /* ADD r/m8, imm8    */ {0x80, 0, MODRM | REG_OPCODE | IMM8},
    /* ADD r/m32, imm32  */ {0x81, 0, MODRM | REG_OPCODE | IMM32},
    /* ADD r/m32, imm8   */ {0x83, 0, MODRM | REG_OPCODE | IMM8},
    /* ADD r/m8, r8      */ {0x00, 0, MODRM},
    /* ADD r/m32, r32    */ {0x01, 0, MODRM},
    /* ADD r8, r/m8      */ {0x02, 0, MODRM},
    /* ADD r32, r/m32    */ {0x03, 0, MODRM},
    /* AND AL, imm8      */ {0x24, 0, IMM8},
    /* AND EAX, imm32    */ {0x25, 0, IMM32},
    /* AND r/m8, imm8    */ {0x80, 4, MODRM | REG_OPCODE | IMM8},
    /* AND r/m32, imm32  */ {0x81, 4, MODRM | REG_OPCODE | IMM32},
    /* AND r/m32, imm8   */ {0x83, 4, MODRM | REG_OPCODE | IMM8},
    /* AND r/m8, r8      */ {0x20, 0, MODRM},
    /* AND r/m32, r32    */ {0x21, 0, MODRM},
    /* AND r8, r/m8      */ {0x22, 0, MODRM},
    /* AND r32, r/m32    */ {0x23, 0, MODRM},
    /* CALL rel32        */ {0xE8, 0, IMM32 | RELOC},
    /* CALL r/m32        */ {0xFF, 2, MODRM | REG_OPCODE},
    /* CMP r/m32, imm8   */ {0x83, 7, MODRM | REG_OPCODE | IMM8},
    /* CMP r/m32, r32    */ {0x39, 0, MODRM},
    /* CMP imm16/32      */ {0x3D, 0, IMM32},
    /* DEC r/m32         */ {0xFF, 1, MODRM | REG_OPCODE},
    /* DEC r32           */ {0x48, 0, PLUS_R},
    /* ENTER imm16, imm8 */ {0xC8, 0, IMM16 | IMM8},
    /* FLD m32fp         */ {0xD9, 0, MODRM | REG_OPCODE},
    /* FLD m64fp         */ {0xDD, 0, MODRM | REG_OPCODE},
    /* FLD m80fp         */ {0xDB, 5, MODRM | REG_OPCODE},
    /* INT 3             */ {0xCC, 0, 0},
    /* JMP rel32         */ {0xE9, 0, IMM32 | RELOC},
    /* JMP r/m32         */ {0xFF, 4, MODRM | REG_OPCODE},
    /* LEA r32,m         */ {0x8D, 0, MODRM},
    /* LEAVE             */ {0xC9, 0, 0},
    /* MOV r/m8,r8       */ {0x88, 0, MODRM},
    /* MOV r/m32,r32     */ {0x89, 0, MODRM},
    /* MOV r8,r/m8       */ {0x8A, 0, MODRM},
    /* MOV r32,r/m32     */ {0x8B, 0, MODRM},
    /* MOV r/m16,Sreg    */ {0x8C, 0, MODRM},
    /* MOV Sreg,r/m16    */ {0x8E, 0, MODRM},
    /* MOV AL,moffs8     */ {0xA0, 0, IMM8},
    /* MOV EAX,moffs32   */ {0xA1, 0, IMM32},
    /* MOV moffs8,AL     */ {0xA2, 0, IMM8},
    /* MOV moffs32,EAX   */ {0xA3, 0, IMM32},
    /* MOV r8, imm8      */ {0xB0, 0, PLUS_R | IMM8},
    /* MOV r32, imm32    */ {0xB8, 0, PLUS_R | IMM32},
    /* MOV r/m8, imm8    */ {0xC6, 0, MODRM | REG_OPCODE | IMM8},
    /* MOV r/m32, imm32  */ {0xC7, 0, MODRM | REG_OPCODE | IMM32},
    /* NOP               */ {0x90, 0, 0},
    /* OR AL, imm8       */ {0x0C, 0, IMM8},
    /* OR EAX, imm32     */ {0x0D, 0, IMM32},
    /* OR r/m8, imm8     */ {0x80, 1, MODRM | REG_OPCODE | IMM8},
    /* OR r/m32, imm32   */ {0x81, 1, MODRM | REG_OPCODE | IMM32},
    /* OR r/m32, imm8    */ {0x83, 1, MODRM | REG_OPCODE | IMM8},
    /* OR r/m8, r8       */ {0x08, 0, MODRM},
    /* OR r/m32, r32     */ {0x09, 0, MODRM},
    /* OR r8, r/m8       */ {0x0A, 0, MODRM},
    /* OR r32, r/m32     */ {0x0B, 0, MODRM},
    /* POP r/m32         */ {0x8F, 0, MODRM | REG_OPCODE},
    /* POP r32           */ {0x58, 0, PLUS_R},
    /* PUSH r/m32        */ {0xFF, 6, MODRM | REG_OPCODE},
    /* PUSH r32          */ {0x50, 0, PLUS_R},
    /* PUSH imm8         */ {0x6A, 0, IMM8},
    /* PUSH imm32        */ {0x68, 0, IMM32},
    /* RET               */ {0xC3, 0, 0},
    /* RET imm16         */ {0xC2, 0, IMM16},
    /* SUB AL, imm8      */ {0x2C, 0, IMM8},
    /* SUB EAX, imm32    */ {0x2D, 0, IMM32},
    /* SUB r/m8, imm8    */ {0x80, 5, MODRM | REG_OPCODE | IMM8},
    /* SUB r/m32, imm32  */ {0x81, 5, MODRM | REG_OPCODE | IMM32},
    /* SUB r/m32, imm8   */ {0x83, 5, MODRM | REG_OPCODE | IMM8},
    /* SUB r/m8, r8      */ {0x28, 0, MODRM},
    /* SUB r/m32, r32    */ {0x29, 0, MODRM},
    /* SUB r8, r/m8      */ {0x2A, 0, MODRM},
    /* SUB r32, r/m32    */ {0x2B, 0, MODRM},
    /* TEST AL, imm8     */ {0xA8, 0, IMM8},
    /* TEST EAX, imm32   */ {0xA9, 0, IMM32},
    /* TEST r/m8, imm8   */ {0xF6, 0, MODRM | REG_OPCODE | IMM8},
    /* TEST r/m32, imm32 */ {0xF7, 0, MODRM | REG_OPCODE | IMM32},
    /* TEST r/m8, r8     */ {0x84, 0, MODRM},
    /* TEST r/m32, r32   */ {0x85, 0, MODRM},
    /* XOR AL, imm8      */ {0x34, 0, IMM8},
    /* XOR EAX, imm32    */ {0x35, 0, IMM32},
    /* XOR r/m8, imm8    */ {0x80, 6, MODRM | REG_OPCODE | IMM8},
    /* XOR r/m32, imm32  */ {0x81, 6, MODRM | REG_OPCODE | IMM32},
    /* XOR r/m32, imm8   */ {0x83, 6, MODRM | REG_OPCODE | IMM8},
    /* XOR r/m8, r8      */ {0x30, 0, MODRM},
    /* XOR r/m32, r32    */ {0x31, 0, MODRM},
    /* XOR r8, r/m8      */ {0x32, 0, MODRM},
    /* XOR r32, r/m32    */ {0x33, 0, MODRM}
  };

  uint8_t *code = (uint8_t *)src;
  size_t i;
  int len = 0;
  int operand_size = 4;
  uint8_t opcode = 0;
  int found_opcode = false;

  for (i = 0; i < sizeof(prefixes) / sizeof(*prefixes); i++) {
    if (code[len] == prefixes[i]) {
      len++;
      if (prefixes[i] == 0x66) {
        operand_size = 2;
      }
    }
  }

#ifdef SUBHOOK_X86_64
  if ((code[len] & 0xF0) == 0x40) {
    /* This is a REX prefix (40H - 4FH). REX prefixes are valid only in
     * 64-bit mode.
     */
    uint8_t rex = code[len++];

    if (rex & 8) {
      // 此处定义的是寄存器是64位，而不是立即数
      /* REX.W changes size of immediate operand to 64 bits. */
      spdlog::debug("set operand_size to 8, pre operand_size: {}", operand_size);
      spdlog::debug("len: {}, rex: {}", len, rex);
      // operand_size = 8;
    }
  }
#endif

  for (i = 0; i < sizeof(opcodes) / sizeof(*opcodes); i++) {
    if (code[len] == opcodes[i].opcode) {
      if (opcodes[i].flags & REG_OPCODE) {
        found_opcode = ((code[len + 1] >> 3) & 7) == opcodes[i].reg_opcode;
      } else {
        found_opcode = true;
      }
    }

    if ((opcodes[i].flags & PLUS_R)
      && (code[len] & 0xF8) == opcodes[i].opcode) {
      found_opcode = true;
    }

    if (found_opcode) {
      opcode = code[len++];
      break;
    }
  }

  if (!found_opcode) {
    return 0;
  }

  if (reloc_op_offset != NULL && opcodes[i].flags & RELOC) {
    /* Either a call or a jump instruction that uses an absolute or relative
     * 32-bit address.
     *
     * Note: We don't support short (8-bit) offsets at the moment, so the
     * caller can assume the operand will be always 4 bytes.
     */
    *reloc_op_offset = len;
  }

  if (opcodes[i].flags & MODRM) {
    uint8_t modrm = code[len++]; /* +1 for Mod/RM byte */
    uint8_t mod = modrm >> 6;
    uint8_t rm = modrm & 0x07;

    if (mod != 3 && rm == 4) {
      uint8_t sib = code[len++]; /* +1 for SIB byte */
      uint8_t base = sib & 0x07;

      if (base == 5) {
        /* The SIB is followed by a disp32 with no base if the MOD is 00B.
         * Otherwise, disp8 or disp32 + [EBP].
         */
        if (mod == 1) {
          len += 1; /* for disp8 */
        } else {
          len += 4; /* for disp32 */
        }
      }
    }

#ifdef SUBHOOK_X86_64
    if (reloc_op_offset != NULL && mod == 0 && rm == 5) {
      /* RIP-relative addressing: target is at [RIP + disp32]. */
      *reloc_op_offset = (int32_t)len;
    }
#endif

    if (mod == 1) {
      len += 1; /* for disp8 */
    }
    if (mod == 2 || (mod == 0 && rm == 5)) {
      len += 4; /* for disp32 */
    }
  }
  spdlog::debug("len: {}", len);
  if (opcodes[i].flags & IMM8) {
    len += 1;
  }
  if (opcodes[i].flags & IMM16) {
    len += 2;
  }
  if (opcodes[i].flags & IMM32) {
    spdlog::debug("imm32! operand_size: {}", operand_size);
    len += operand_size;
  }
  spdlog::debug("custom disasm result: {}, reloc_op_offset: {}", len, *reloc_op_offset);
  return len;
}
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

  subhook_set_disasm_handler(custom_disasm);

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