
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <sqlite3.h>
#include "../include/handle.hh"
#include "../include/sqlite3/nt/vdbe.hh"
#include "../include/sqlite3/nt/sqlite3.hh"
#include "../include/sqlite3/nt/base.hh"
#include "../include/sqlite3/official/base.hh"
#include "../include/sqlite3/official/sqlite3.hh"
#include "../include/sqlite3/official/vdbe.hh"

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

namespace yukihana {
  const char* db_name = "/home/msojocs/ntqq/nt-native/test/test.db";
  std::shared_ptr<NTNative::LinuxHook> hook;
  std::map<void *, sqlite3_stmt *> nt2custom;
  
  typedef int (*stmt_func)(void * 
  ,void *
  ,void *  
  ,void *  
  ,void * 
  ,void *  
  //  ,int 
  );

  NTMem * conver2NTMem(Mem *row, int count) {
    NTMem * result = (NTMem *)malloc(sizeof(NTMem) * count);
    for (int i=0; i<count; i++) {
      Mem cur = row[i];
      NTMem target = result[i];
      if (cur.flags & MEM_Int) {
        // 数值
        target.flags = MEM_Int;
        target.u.i = cur.u.i;
      }
      else if (cur.flags & MEM_Str) {
        // 字符串
        target.flags = MEM_Str;
        target.enc = cur.enc;
        target.z = cur.z;
        target.zMalloc = cur.zMalloc;
      }
      else if (cur.flags & MEM_Blob) {
        // 二进制
        target.flags = MEM_Blob;
        target.z = cur.z;
        target.enc = cur.enc;
        target.zMalloc = cur.zMalloc;
      }
      else {
        printf("not supported!\n");
      }
    }
    return result;
  }

  int execute(void* a1,
  void* a2,
  void* a3,
  void* a4,
  void* a5,
  void* a6) {
    
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
    // printf("[%s] sql: %s\n", u, sqlite3_sql(ntStmt));
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
      int rc = sqlite3_step(customStmt);
      if (rc == SQLITE_ROW) {
        // TODO:对malloc的内存进行free
        int colCount = sqlite3_column_count(customStmt);
        auto newRow = conver2NTMem(((Vdbe *)customStmt)->pResultRow, colCount);
        ntVdbe->pResultRow = newRow;
      }
      else {
        sqlite3_finalize(customStmt);
        sqlite3_close(*(sqlite3 **)customStmt);
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
      // if (ntVdbe->zSql != nullptr) {
      //   std::string sql(ntVdbe->zSql);
      //   if (sql.find("FROM group_msg_table") != std::string::npos) {
      //     printf("replace row data\n");
      //     // 还有数据，直接返回
      //     // printRow(ntStmt);
      //     // ntVdbe->pResultRow = rowTest;
      //   }
      // }
      return ret;
    }
    // 3. 非SQLITE_ROW继续
    
    // std::cout << "foo(" << (char *)a << "," << *(char **)b << "," << *(char **)c  << "," << (char *)d << ") called" << std::endl;
    printf("stmt(...) called\n");;
    if (ntStmt != nullptr)
    {
        printf("actual sql0 -> %s\n", *((const char **)a1 + 32));
        printf("read from v\n");
        // 4. 取sql执行
        printf("actual sql1 -> %s\n", ntVdbe->zSql);
        if (ntVdbe->zSql == nullptr) {
            return ret;
        }
        // 5. 有数据就替换stmt的row内容，并返回SQLITE_ROW
        std::string sql(ntVdbe->zSql);
        printf("sql1:%s\n", sql.c_str());
        printf("ret:%d\n", ret);
        if (sql.find("FROM group_msg_table") != std::string::npos) {
            printf("try to read record from custom db!\n");
            sqlite3 *db = nullptr;
            const char *vfs = nullptr;
            int rc = 0;
            printf("try to open custom db!\n");
            rc = sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, vfs);
            if (rc != SQLITE_OK)
            {
                sqlite3_close(db);
                printf("failed to open db!\n");
                return ret;
            }

            // Now we create an SQL command which is stored in an sqlite3_stmt data structure.
            // Note symColName_ is a member of EquityDataLocator
            sqlite3_stmt *newStmt = nullptr;
            printf("try to prepare sql[%ld]: %s\n", strlen(ntVdbe->zSql), ntVdbe->zSql);
            rc = sqlite3_prepare_v2(db, ntVdbe->zSql, strlen(ntVdbe->zSql), &newStmt, nullptr);
            if (rc != SQLITE_OK)
            {
                sqlite3_finalize(newStmt);
                sqlite3_close(db);
                printf("failed to prepare sql!\n");
                return ret;
            }
            auto querySql = sqlite3_sql(newStmt);
            printf("sql:%s\n", querySql);
            // Vdbe * info = (Vdbe *)stmt;
            
            // Now we retrieve the row
            rc = sqlite3_step(newStmt);
            if (rc == SQLITE_ROW)
            {
              printf("has external data!!\n");
              // Here we get a pointer to the location text ( stored in the second column of the table )
              // The 1 in sqlite3_column_text( stmt, 1 ) is the column number (zero based).
              // sqlite3_column_text( sqlite_stmt* stmt, int cidx ) returns const unsigned char* so the casts are necessary.
              void *p = const_cast<unsigned char *>(sqlite3_column_text(newStmt, 1));
              const char *ret = static_cast<const char *>(p);
              printf("data:%s\n", ret);

              auto row = ((Vdbe *)newStmt)->pResultRow;
              int colCount = sqlite3_column_count(newStmt);
              // int colCount = ntVdbe->nResColumn;
              printf("column num: %d\n", colCount);
              auto newRow = conver2NTMem(row, colCount);

              printf("[%s]end!!!\n", u);
              ntVdbe->pResultRow = newRow;
              // return SQLITE_ROW;
              // 成功
              nt2custom.insert(std::pair<void *, sqlite3_stmt *>(a1, newStmt));
              return rc;
            }
            else
            {
              printf("close db...\n");
              sqlite3_finalize(newStmt);
              sqlite3_close(db);
              return rc;
            }
        }
        // sqlite3_finalize(stmt);
        // sqlite3_close(db);
        // 6. 没数据就返回原值
    }
    
    printf("[%s]result: %d\n", u, ret);
    return SQLITE_DONE;

  }
}