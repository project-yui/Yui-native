#include "include/linux_hook.hh"
#include "include/sqlite3/base.hh"
#include "include/sqlite3/vdbe.hh"
#include "include/sqlite3/sqlite3.hh"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <random>
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
    sqlite3_stmt *stmt0 = nullptr;
    stmt0 = (sqlite3_stmt *)a1;
    NTVdbe * v = (NTVdbe *)stmt0;
    printf("[%s] sql: %s\n", u, _sqlite3_sql(stmt0));
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
      int rc = _sqlite3_step(customStmt);
      if (rc == SQLITE_ROW) {
        // 还有数据，读取并转换
      }
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
      if (v->zSql != nullptr) {
        std::string sql(v->zSql);
        if (sql.find("FROM group_msg_table") != std::string::npos) {
          // 还有数据，直接返回
          printRow(stmt0);
        }
      }
      return ret;
    }
    return ret;
    // 3. 非SQLITE_ROW继续
    
    // std::cout << "foo(" << (char *)a << "," << *(char **)b << "," << *(char **)c  << "," << (char *)d << ") called" << std::endl;
    std::cout << "stmt(..." << ") called" << std::endl;
    if (stmt0 != nullptr)
    {
        printf("actual sql0 -> %s\n", *((const char **)a1 + 32));
        printf("read from v\n");
        // 4. 取sql执行
        std::cout << "sql0:" << v->zSql << std::endl;
        printf("actual sql1 -> %s\n", v->zSql);
        if (v->zSql == nullptr) {
            return ret;
        }
        // 5. 有数据就替换stmt的row内容，并返回SQLITE_ROW
        std::string sql(v->zSql);
        printf("sql1:%s\n", sql.c_str());
        printf("ret:%d\n", ret);
        if (sql.find("FROM group_msg_table") != std::string::npos) {
            // printRow(stmt0);
            return ret;
            if (!isTest) {
              return SQLITE_DONE;
            }
            _sqlite3_initialize();
            sql_global = v->zSql;
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
            printf("try to prepare sql[%ld]: %s\n", strlen(v->zSql), v->zSql);
            rc = _sqlite3_prepare_v2(db, v->zSql, strlen(v->zSql), &newStmt, nullptr);
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
                int colCount = v->nResColumn;
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