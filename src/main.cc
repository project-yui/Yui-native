#include "include/linux_hook.hh"
#include "include/nt/element.hh"
#include "include/nt/message.hh"
#include "include/nt_sqlite3/base.hh"
#include "include/nt_sqlite3/vdbe.hh"
#include "include/nt_sqlite3/sqlite3.hh"
#include "include/nt_sqlite3/base.hh"
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

#define BUF_SIZE 1024
namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}

typedef int (*stmt_func)(void * 
,void *
 ,void *  
 ,void *  
 ,void * 
 ,void *  
//  ,int 
);

std::shared_ptr<NTNative::LinuxHook> hook;
std::map<void *, sqlite3_stmt *> nt2custom;

bool isTest = true;
char * sql_global = nullptr;

void printValue(const char* name, unsigned long long v) {
  printf("[%s] %ld, %llx\n", name, v, v);
}
void printHex(char* addr, int size) {
  for (int i=0; i < size; i++) {
    printf("%X", addr[i]);
    if (i % 4 == 3) {
      printf(" ");
    }
  }
  printf("\n");
  
}
void printRow(sqlite3_stmt * stmt) {
  int colCount = _sqlite3_column_count(stmt);
  NTVdbe * v = (NTVdbe *)stmt;
  printf("column: %d, %d\n", colCount, v->nResColumn);
  printf("db address: %lx\n", v->db);
  // long msgId = _sqlite3_column_int64(stmt, 0);
  // printf("msgId: %ld\n", msgId);

  for (int i=0; i < colCount; i++) {
    NTMem r = v->pResultRow[i];
    printHex((char *)&r, 70);
    printValue("u", (unsigned long long) r.u.i);
    printValue("z", (unsigned long long) r.z);
    printValue("n", (unsigned long long) r.n);
    printValue("flags", (unsigned long long) r.flags);
    printValue("enc", (unsigned long long) r.enc);
    printValue("eSubtype", (unsigned long long) r.eSubtype);
    printValue("db", (unsigned long long) r.db);
    printValue("szMalloc", (unsigned long long) r.szMalloc);
    printValue("uTemp", (unsigned long long) r.uTemp);
    printValue("zMalloc", (unsigned long long) r.zMalloc);
    // printValue("xDel", (unsigned long long) r.xDel);
    // printValue("pScopyFrom", (unsigned long) data.pScopyFrom);
    // printValue("mScopyFlags", (unsigned long) data.mScopyFlags);
    printf("=====================\n");
  }

}
u8 data_1[] = {0x82, 0xf6, 0x13, 0x29, 0xc8, 0xfc, 0x15, 0x97, 0x84, 0x8e, 0xdb, 0xa7, 0xab, 0xd8, 0xec, 0x65, 0xd0, 0xfc, 0x15, 0x01, 0xea, 0x82, 0x16, 0x11, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0xf0, 0x82, 0x16, 0x00};
u8 data_2[] = {0xc2, 0xe9, 0x13, 0x04, 0xa8, 0xd1, 0x14, 0x00};
u8 data_3[] = {0x8a, 0xf6, 0x13, 0x4a, 0xe8, 0xa1, 0x14, 0x96, 0x84, 0x8e, 0xdb, 0xa7, 0xab, 0xd8, 0xec, 0x65, 0xf0, 0xa1, 0x14, 0x00, 0xb0, 0xa2, 0x14, 0x00, 0xb8, 0xa2, 0x14, 0xe1, 0xb1, 0xe4, 0xba, 0x05, 0xf8, 0xa2, 0x14, 0x00, 0x80, 0xa3, 0x14, 0x00, 0x90, 0xa3, 0x14, 0x00, 0x98, 0xa3, 0x14, 0x00, 0xa0, 0xa3, 0x14, 0x00, 0xa8, 0xa3, 0x14, 0x00, 0xc8, 0xa3, 0x14, 0x00, 0xd0, 0xa3, 0x14, 0x00, 0xd8, 0xa3, 0x14, 0xa7, 0xc0, 0x04, 0xe8, 0xa3, 0x14, 0x00, 0xf8, 0xa3, 0x14, 0x00};
char uid[] = "u_K54_tDilsiaIV_m0q4XgCg";
char nickName[] = "msojocs";
char groupId[] = "933286835";
NTMem rowTest[33] = {
  // INSERT INTO "main"."group_msg_table" ("40001", "40002", "40003", "40010", "40011", "40012", "40013", "40020", "40026", "40021", "40027", "40040", "40041", "40050", "40052", "40090", "40093", "40800", "40900", "40105", "40005", "40058", "40006", "40100", "40600", "40060", "40850", "40851", "40601", "40801", "40605", "40030", "40033") VALUES ('7339004109022003736', '3581593724', '9991', '2', '2', '1', '0', 'u_K54_tDilsiaIV_m0q4XgCg', '1', '933286835', '933286835', '0', '2', '1708603154', '0', '', 'msojocs', X'82f61329c8fc1597848edba7abd8ec65d0fc1501ea8216113737363737363737363737363737363737f0821600', '', '0', '0', '1708531200', '0', '0', X'c2e91304a8d11400', '0', '0', '0', '', X'8af6134ae8a11496848edba7abd8ec65f0a11400b0a21400b8a214e1b1e4ba05f8a2140080a3140090a3140098a31400a0a31400a8a31400c8a31400d0a31400d8a314a7c004e8a31400f8a31400', '', '933286835', '1690127128');
	// "40001"	INTEGER,
  {
    7337665346128031342ul,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40002"	INTEGER,
  {
    3581593724,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40003"	INTEGER,
  {
    9991,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40010"	INTEGER,
  {
    2,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40011"	INTEGER,
  {
    2,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40012"	INTEGER,
  {
    1,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40013"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40020"	TEXT,
  {
    0,
    MEM_Str | MEM_Term,
    SQLITE_UTF8,
    0,
    24,
    uid,
    uid
  },
	// "40026"	INTEGER,
  {
    1,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40021"	TEXT,
  {
    1,
    MEM_Str,
    SQLITE_UTF8,
    0,
    9,
    groupId,
    groupId
  },
	// "40027"	INTEGER,
  {
    933286835,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40040"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40041"	INTEGER,
  {
    2,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40050"	INTEGER,
  {
    1708603154,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40052"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40090"	TEXT,
  {
    0,
    MEM_Str,
    SQLITE_UTF8,
    0,
    0,
    "",
    ""
  },
	// "40093"	TEXT,
  {
    0,
    MEM_Str,
    SQLITE_UTF8,
    0,
    7,
    nickName,
    nickName
  },
	// "40800"	BLOB,
  {
    0,
    MEM_Blob,
    SQLITE_UTF8,
    0,
    45,
    (char *)data_1,
    (char *)data_1
  },
	// "40900"	BLOB,
  {
    0,
    MEM_Blob,
    SQLITE_UTF8,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40105"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40005"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40058"	INTEGER,
  {
    1708531200,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40006"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40100"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40600"	BLOB,
  {
    0,
    MEM_Blob,
    SQLITE_UTF8,
    0,
    8,
    (char *)data_2,
    (char *)data_2
  },
	// "40060"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40850"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40851"	INTEGER,
  {
    0,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40601"	BLOB,
  {
    0,
    MEM_Blob,
    SQLITE_UTF8,
    0,
    0,
    "",
    ""
  },
	// "40801"	BLOB,
  {
    0,
    MEM_Blob,
    SQLITE_UTF8,
    0,
    78,
    (char *)data_3,
    (char *)data_3
  },
	// "40605"	BLOB,
  {
    0,
    MEM_Blob,
    SQLITE_UTF8,
    0,
    0,
    "",
    ""
  },
	// "40030"	INTEGER,
  {
    933286835,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },
	// "40033"	INTEGER,
  {
    1690127128,
    MEM_Int,
    0,
    0,
    0,
    nullptr,
    nullptr
  },

};

Message msg;

void convertELements2buf(std::vector<Element>& elems, std::string &output) {
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
void init() {
  msg.msgId = 7040191714128545809;
  msg.msgRandom = 123456;
  msg.msgSeq = 66;
  msg.cntSeq = 0;
  msg.chatType = 2;
  msg.msgType = 2;
  msg.subMsgType = 1;
  msg.senderUid = "u_K54_tDilsiaIV_m0q4XgCg";
  msg.peerUid = "933286835";
  msg.msgTime = 1708603150;
  msg.sendStatus = 2;
  msg.sendNickName = "msojocs";
  msg.peerName = "测试";
  msg.senderUin = "1690127128";
  msg.peerUin = "933286835";
  TextElemnt text;
  text.content = "test test";
  Element ele = {
    1,
    7040191714128545808,
    "0x",
    text,
  };
  msg.elements.push_back(ele);

  std::string msgContext;
  convertELements2buf(msg.elements, msgContext);

  char * data = (char *)malloc(msgContext.length() + 1);
  memset(data, 0, msgContext.length() + 1);
  memcpy(data, msgContext.c_str(), msgContext.length());
  rowTest[17] = {
    0,
    MEM_Blob,
    SQLITE_UTF8,
    0,
    (int)msgContext.length(),
    (char *)data,
    (char *)data
  };
}

int execute(void * a1
, void * a2
, void *  a3
, void *  a4
, void *  a5
, void *  a6
// , int a7
) {
    std::string uuid = uuid::generate_uuid_v4();
    const char * u = uuid.c_str();
    printf("[%s]execute\n", u);
    stmt_func fun = (stmt_func)hook->get_trampoline();
    if (fun == nullptr) {
        printf("[%s]error nullptr!!!\n", u);
        return -1;
    }
    sqlite3_stmt *ntStmt = (sqlite3_stmt *)a1;
    NTVdbe * ntVdbe = (NTVdbe *)ntStmt;
    printf("[%s] sql: %s\n", u, _sqlite3_sql(ntStmt));
    // if (sql_global != nullptr) {
    //   printf("[%s]compare%x <-> %x!!!\n", u, sql_global, v->zSql);
    //   if (sql_global == v->zSql) {
    //     printf("[%s]equal!!!\n", u);
    //     return SQLITE_DONE;
    //   }
    // }
    // 替换
    if (nt2custom.find(a1) != nt2custom.end()) {
      sqlite3_stmt * customStmt = nt2custom[a1];
      // 找到sql实例
      // int rc = _sqlite3_step(customStmt);
      // if (rc == SQLITE_ROW) {
      //   // 还有数据，读取并转换
      //   int colCOunt = _sqlite3_column_count(customStmt);
      //   NTMem * row = (NTMem *)malloc(colCOunt * sizeof(NTMem));
      //   // ntVdbe->pResultRow = row;
      //   for (int i=0; i < colCOunt; i++) {
      //     // row[i].db = 
      //   }
      // }
      // else if (rc == SQLITE_DONE) {
      //   // _sqlite3_close(sqlite3 *)
      // }
      return SQLITE_DONE;
    }
    // 1. 执行原来的调用
    int ret = fun(a1
        , a2
        , a3
        , a4
        , a5
        , a6
    );
    // 2. SQLITE_ROW就返回
    if (ret == SQLITE_ROW) {
      if (ntVdbe->zSql != nullptr) {
        std::string sql(ntVdbe->zSql);
        if (sql.find("FROM group_msg_table") != std::string::npos) {
          printf("replace row data\n");
          // 还有数据，直接返回
          // printRow(ntStmt);
          ntVdbe->pResultRow = rowTest;
        }
      }
      return ret;
    }
    return ret;
    // 3. 非SQLITE_ROW继续
    
    // std::cout << "foo(" << (char *)a << "," << *(char **)b << "," << *(char **)c  << "," << (char *)d << ") called" << std::endl;
    std::cout << "stmt(..." << ") called" << std::endl;
    if (ntStmt != nullptr)
    {
        printf("actual sql0 -> %s\n", *((const char **)a1 + 32));
        printf("read from v\n");
        // 4. 取sql执行
        std::cout << "sql0:" << ntVdbe->zSql << std::endl;
        printf("actual sql1 -> %s\n", ntVdbe->zSql);
        if (ntVdbe->zSql == nullptr) {
            return ret;
        }
        // 5. 有数据就替换stmt的row内容，并返回SQLITE_ROW
        std::string sql(ntVdbe->zSql);
        printf("sql1:%s\n", sql.c_str());
        printf("ret:%d\n", ret);
        if (sql.find("FROM group_msg_table") != std::string::npos) {
            // printRow(stmt0);
            return ret;
            if (!isTest) {
              return SQLITE_DONE;
            }
            _sqlite3_initialize();
            sql_global = ntVdbe->zSql;
            isTest = false;
            printf("try to read record from custom db!\n");
            sqlite3 *db = nullptr;
            const char *vfs = nullptr;
            int rc = 0;
            printf("try to open custom db!\n");
            rc = _sqlite3_open_v2("./test/test.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, vfs);
            if (rc != SQLITE_OK)
            {
                _sqlite3_close(db);
                printf("failed to open db!\n");
                return ret;
            }

            // Now we create an SQL command which is stored in an sqlite3_stmt data structure.
            // Note symColName_ is a member of EquityDataLocator
            sqlite3_stmt *newStmt = nullptr;
            printf("try to prepare sql[%ld]: %s\n", strlen(ntVdbe->zSql), ntVdbe->zSql);
            rc = _sqlite3_prepare_v2(db, ntVdbe->zSql, strlen(ntVdbe->zSql), &newStmt, nullptr);
            if (rc != SQLITE_OK)
            {
                _sqlite3_finalize(newStmt);
                _sqlite3_close(db);
                printf("failed to prepare sql!\n");
                return ret;
            }
            auto querySql = _sqlite3_sql(newStmt);
            printf("sql:%s\n", querySql);
            // Vdbe * info = (Vdbe *)stmt;
            
            // Now we retrieve the row
            rc = _sqlite3_step(newStmt);
            if (rc == SQLITE_ROW)
            {
                // Here we get a pointer to the location text ( stored in the second column of the table )
                // The 1 in sqlite3_column_text( stmt, 1 ) is the column number (zero based).
                // sqlite3_column_text( sqlite_stmt* stmt, int cidx ) returns const unsigned char* so the casts are necessary.
                void *p = const_cast<unsigned char *>(_sqlite3_column_text(newStmt, 1));
                const char *ret = static_cast<const char *>(p);
                printf("data:%s\n", ret);
                int colCount = ntVdbe->nResColumn;
                printf("column num: %d\n", colCount);

                auto result = ((NTVdbe *)newStmt)->pResultRow;
                for (int i=0; i < colCount; i++) {
                  auto data = result[i];

                  printValue("u", (unsigned long) data.u.i);
                  printValue("z", (unsigned long) data.z);
                  printValue("n", (unsigned long) data.n);
                  printValue("flags", (unsigned long) data.flags);
                  printValue("enc", (unsigned long) data.enc);
                  printValue("eSubtype", (unsigned long) data.eSubtype);
                  printValue("db", (unsigned long) data.db);
                  printValue("szMalloc", (unsigned long) data.szMalloc);
                  printValue("uTemp", (unsigned long) data.uTemp);
                  printValue("zMalloc", (unsigned long) data.zMalloc);
                  // printValue("xDel", (unsigned long) data.xDel);
                  // printValue("pScopyFrom", (unsigned long) data.pScopyFrom);
                  // printValue("mScopyFlags", (unsigned long) data.mScopyFlags);
                  printf("=====================\n");
                }


                printf("[%s]end!!!\n", u);
                // v->pResultRow = result;
                // return SQLITE_ROW;
                // 成功
                nt2custom.insert(std::pair<void *, sqlite3_stmt *>(a1, newStmt));
                return rc;
            }
            else
            {
                _sqlite3_finalize(newStmt);
                _sqlite3_close(db);
                return ret;
            }
        }
        // sqlite3_finalize(stmt);
        // sqlite3_close(db);
        // 6. 没数据就返回原值
    }
    
    printf("[%s]result: %d\n", u, ret);
    return SQLITE_DONE;

}

// 特征码
// const std::vector<uint8_t> feature_code = { 0x41, 0x57, 0x41, 0x56, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x49, 0x89, 0xFF, 0x48, 0x8B, 0x02, 0x48, 0x89, 0x44, 0x24, 0x10, 0xC7, 0x44, 0x24, 0x28, 0x0B, 0x00, 0x00, 0x00, 0xF6, 0x01, 0x01, 0x74, 0x06, 0x48, 0x8B, 0x49, 0x10, 0xEB, 0x03, 0x48, 0xFF, 0xC1, 0x4C, 0x8D, 0x74, 0x24, 0x30, 0x48, 0x89, 0x4C, 0x24, 0x30, 0xC7, 0x44, 0x24, 0x48, 0x0B, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x06, 0x48, 0x8B, 0x56, 0x08, 0x48, 0x8D, 0x4C, 0x24, 0x10, 0x41, 0xB8, 0x02, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xFF, 0x48, 0x89, 0xC6, 0xE8, 0x37, 0x0F, 0x9E, 0x03, 0x8B, 0x44, 0x24, 0x48, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0x48, 0x39, 0xD8, 0x74, 0x12, 0x48, 0x8D, 0x0D, 0x2A, 0x82, 0xCB, 0x03, 0x48, 0x8D, 0x7C, 0x24, 0x08, 0x4C, 0x89, 0xF6, 0xFF, 0x14, 0xC1, 0xC7, 0x44, 0x24, 0x48, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x44, 0x24, 0x28, 0x48, 0x39, 0xD8, 0x74, 0x14, 0x48, 0x8D, 0x0D, 0x07, 0x82, 0xCB };
// const std::vector<uint8_t> feature_code = { 0x55, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x53, 0x48, 0x83, 0xEC, 0x18, 0x48, 0x85, 0xFF, 0x0F, 0x84, 0x96, 0x03, 0x00, 0x00, 0x49, 0x89, 0xFC, 0x48, 0x8B, 0x07, 0x48, 0x85, 0xC0 };

void hook2(std::vector<uint8_t> & feature_code) {
    std::cout << "Hello, world!\n";
    
    std::string target = "wrapper.node";
    init();
    _sqlite3_initialize();
    // printf("pid of this process:%d\n", pid);
    // getNameByPid(pid, task_name);

    /*
    strcpy(task_name, argv[0]+2);
    printf("task name is %s\n", task_name);
    getPidByName(task_name);
    */
    pid_t p = getpid();
    printf("current pid:%d\n", p);
    hook.reset(new NTNative::LinuxHook(p, target));

    printf("set_signature\n");
    hook->set_signature(feature_code);

    printf("install\n");
    hook->install((void *)execute);
}
static Napi::Boolean test(const Napi::CallbackInfo &info) {
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
  hook2(code);
  return Napi::Boolean::New(env, true);
}
static Napi::Boolean test2(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto failed = Napi::Boolean::New(env, false);
  // sqlite测试
  std::cout << "test sqlite3" << std::endl;
  int rc = 0;
  rc = _sqlite3_initialize(); // Initializes the library. If the library has already been initialized it has no effect.
  if (rc != SQLITE_OK)
  {
    std::cout << "failed to init sqlite3!" << std::endl;
    return failed;
  }
  

  sqlite3 *db = nullptr;
  const char *vfs = nullptr;
  rc = _sqlite3_open_v2("./test/test.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, vfs);
  if (rc != SQLITE_OK)
  {
    _sqlite3_close(db);
    std::cout << "failed to open db!" << std::endl;
    return failed;
  }

  // Now we create an SQL command which is stored in an sqlite3_stmt data structure.
  // Note symColName_ is a member of EquityDataLocator
  sqlite3_stmt *stmt = nullptr;
  std::string s = "SELECT * FROM group_msg_table WHERE \"40027\" = 933286835;";
  rc = _sqlite3_prepare_v2(db, s.c_str(), s.size() + 1, &stmt, nullptr);
  if (rc != SQLITE_OK)
  {
    _sqlite3_finalize(stmt);
    _sqlite3_close(db);
    std::cout << "failed to prepare sql!" << std::endl;
    return failed;
  }

  
  auto querySql = _sqlite3_sql(stmt);
  std::cout << "sql:" << querySql << std::endl;
  // Vdbe * info = (Vdbe *)stmt;
  
  // Now we retrieve the row
  rc = _sqlite3_step(stmt);
  if (rc == SQLITE_ROW)
  {
    // Here we get a pointer to the location text ( stored in the second column of the table )
    // The 1 in sqlite3_column_text( stmt, 1 ) is the column number (zero based).
    // sqlite3_column_text( sqlite_stmt* stmt, int cidx ) returns const unsigned char* so the casts are necessary.
    void *p = const_cast<unsigned char *>(_sqlite3_column_text(stmt, 1));
    const char *ret = static_cast<const char *>(p);
    std::cout << "data:" << ret << std::endl;
  }
  else
  {
    _sqlite3_finalize(stmt);
    _sqlite3_close(db);
    return failed;
  }

  _sqlite3_finalize(stmt);
  _sqlite3_close(db);
  return Napi::Boolean::New(env, true);
}
static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  // https://github.com/chrononeko/bugtracker/issues/7
  // freopen_s(reinterpret_cast<FILE **> stdout, "CONOUT$", "w", stdout);

  exports.Set(
      Napi::String::New(env, "test"),
      Napi::Function::New(env, test));
  exports.Set(
      Napi::String::New(env, "test2"),
      Napi::Function::New(env, test2));

//   exports.Set(
//       Napi::String::New(env, "setPBPreprocessorForGzHook"),
//       Napi::Function::New(env, setPBPreprocessorForGzHook));

//   exports.Set(
//       Napi::String::New(env, "setHandlerForPokeHook"),
//       Napi::Function::New(env, setHandlerForPokeHook));

  return exports;
}

NODE_API_MODULE(cmnative, Init)