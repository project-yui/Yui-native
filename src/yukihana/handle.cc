
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <sqlite3.h>
#include <string>
#include <unistd.h>
#include <utility>
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
  u8 data_1[] = {0x82, 0xf6, 0x13, 0x29, 0xc8, 0xfc, 0x15, 0x97, 0x84, 0x8e, 0xdb, 0xa7, 0xab, 0xd8, 0xec, 0x65, 0xd0, 0xfc, 0x15, 0x01, 0xea, 0x82, 0x16, 0x11, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0xf0, 0x82, 0x16, 0x00};
  u8 data_2[] = {0xc2, 0xe9, 0x13, 0x04, 0xa8, 0xd1, 0x14, 0x00};
  u8 data_3[] = {0x8a, 0xf6, 0x13, 0x4a, 0xe8, 0xa1, 0x14, 0x96, 0x84, 0x8e, 0xdb, 0xa7, 0xab, 0xd8, 0xec, 0x65, 0xf0, 0xa1, 0x14, 0x00, 0xb0, 0xa2, 0x14, 0x00, 0xb8, 0xa2, 0x14, 0xe1, 0xb1, 0xe4, 0xba, 0x05, 0xf8, 0xa2, 0x14, 0x00, 0x80, 0xa3, 0x14, 0x00, 0x90, 0xa3, 0x14, 0x00, 0x98, 0xa3, 0x14, 0x00, 0xa0, 0xa3, 0x14, 0x00, 0xa8, 0xa3, 0x14, 0x00, 0xc8, 0xa3, 0x14, 0x00, 0xd0, 0xa3, 0x14, 0x00, 0xd8, 0xa3, 0x14, 0xa7, 0xc0, 0x04, 0xe8, 0xa3, 0x14, 0x00, 0xf8, 0xa3, 0x14, 0x00};

  struct CustomQuery {
    sqlite3_stmt * stmt;
    NTMem * row;
  };
  std::shared_ptr<NTNative::LinuxHook> hook;
  std::map<sqlite3_stmt *, CustomQuery> nt2custom;
  
  typedef int (*stmt_func)(void * 
  ,void *
  ,void *  
  ,void *  
  ,void * 
  ,void *  
  //  ,int 
  );
  NTMem * conver2NTMem(Vdbe *src, NTVdbe *dest) {
    printf("conver2NTMem: %d\n", src->nResColumn);
    // sleep(2);
    // return rowTest;
    std::map<std::string, Mem*> srcData;
    for (int i=0; i < src->nResColumn; i++) {
      auto col = src->aColName[i];
      auto data = src->pResultRow +i;
      printf("column name length: %d\n", col.n);
      printf("column name: %s\n", col.z);
      std::string name(col.z);
      srcData.emplace(std::pair<std::string, Mem *>(name, data));
    }
    
    printf("malloc column:%d\n", dest->nResColumn);
    NTMem * result = (NTMem *)malloc(sizeof(NTMem) * dest->nResColumn);

    printf("start copy data\n");
    for (int i=0; i < dest->nResColumn; i++) {
      auto col = dest->aColName[i];
      printf("[%s]copy data: %d\n", col.z, i);
      std::string name(col.z);
      NTMem* target = result + i;
      
      target->u.i = 0;
      target->flags = MEM_Null;
      target->enc = 0;
      target->eSubtype = 0;
      target->n = 0;
      target->z = "";
      target->zMalloc = "";
      if (srcData.find(name) == srcData.end()) {
        // if (name == "40600") {
        //   printf("special 40600\n");
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 8;
        //   target->z = (char *)data_2;
        //   target->zMalloc = (char *)data_2;
        //   continue;
        // }
        // else if(name == "40801") {
        //   printf("special 40801\n");
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 78;
        //   target->z = (char *)data_3;
        //   target->zMalloc = (char *)data_3;
        //   continue;
        // }
        // //
        // else if(name == "40040") {
        //   printf("special 40040\n");
        //   target->u.i = 0;
        //   target->flags = MEM_Int;
        //   target->enc = 0;
        //   target->eSubtype = 0;
        //   target->n = 0;
        //   target->z = nullptr;
        //   target->zMalloc = nullptr;
        //   continue;
        // }
        // else if(name == "40052") {
        //   printf("special 40052\n");
        //   target->u.i = 0;
        //   target->flags = MEM_Int;
        //   target->enc = 0;
        //   target->eSubtype = 0;
        //   target->n = 0;
        //   target->z = nullptr;
        //   target->zMalloc = nullptr;
        //   continue;
        // }
        // else if(name == "40900") {
        //   printf("special 40900\n");
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 0;
        //   target->z = "";
        //   target->zMalloc = "";
        //   continue;
        // }
        // else if(name == "40006") {
        //   printf("special 40006\n");
        //   target->u.i = 0;
        //   target->flags = MEM_Int;
        //   target->enc = 0;
        //   target->n = 0;
        //   target->z = nullptr;
        //   target->zMalloc = nullptr;
        //   continue;
        // }
        // else if(name == "40060") {
        //   printf("special 40060\n");
        //   target->u.i = 0;
        //   target->flags = MEM_Int;
        //   target->enc = 0;
        //   target->n = 0;
        //   target->z = nullptr;
        //   target->zMalloc = nullptr;
        //   continue;
        // }
        // else if(name == "40851") {
        //   printf("special 40851\n");
        //   target->u.i = 0;
        //   target->flags = MEM_Int;
        //   target->enc = 0;
        //   target->n = 0;
        //   target->z = nullptr;
        //   target->zMalloc = nullptr;
        //   continue;
        // }
        // else if(name == "40601") {
        //   printf("special 40601\n");
        //   target->u.i = 0;
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 0;
        //   target->z = "";
        //   target->zMalloc = "";
        //   continue;
        // }
        // else if(name == "40605") {
        //   printf("special 40605\n");
        //   target->u.i = 0;
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 0;
        //   target->z = "";
        //   target->zMalloc = "";
        //   continue;
        // }
        printf("not found : %s\n", name.c_str());
        target->n = 0;
        target->flags = MEM_Null;
        target->z = target->zMalloc = "";
        continue;
      }

      Mem * cur = srcData[name];
      if (cur->flags & MEM_Int) {
        // 数值
        target->flags = MEM_Int;
        target->u.i = cur->u.i;
        printf("MEM_Int: %ld\n", cur->u.i);
      }
      else if (cur->flags & (MEM_Str | MEM_Term)) {
        // 字符串
        target->flags = MEM_Str | MEM_Term;
        target->enc = SQLITE_UTF8;
        target->n = cur->n;
        target->z = cur->z;
        target->zMalloc = cur->zMalloc;
        printf("MEM_Str[%d]: %s\n", target->n, cur->z);
      }
      else if (cur->flags & MEM_Blob) {
        // 二进制
        target->flags = MEM_Blob;
        target->z = cur->z;
        target->n = cur->n;
        target->enc = SQLITE_UTF8;
        target->zMalloc = cur->zMalloc;
        printf("MEM_Blob[%d]\n", target->n);
        // *target = rowTest[i];
      }
      else {
        printf("not supported!\n");
        target->z = nullptr;
        target->flags = MEM_Null;
        target->zMalloc = nullptr;
        target->n = 0;
        target->u.i = 0;
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
    
    printf("[%s]try to find handle\n", u);
    if (ntStmt != nullptr) {
      // 替换
      if (nt2custom.find(ntStmt) != nt2custom.end()) {
        auto customQuery = nt2custom[ntStmt];
        // 找到sql实例
        int rc = sqlite3_step(customQuery.stmt);
        if (rc == SQLITE_ROW) {
          printf("continue copy row, free old row\n");
          free(customQuery.row);
          int colCount = sqlite3_column_count(customQuery.stmt);
          auto newRow = conver2NTMem((Vdbe *)customQuery.stmt, ntVdbe);
          ntVdbe->pResultRow = newRow;
          customQuery.row = newRow;
          return SQLITE_ROW;
        }
        else {
          printf("no row to copy, free old row and close db.\n");
          free(customQuery.row);
          sqlite3_finalize(customQuery.stmt);
          sqlite3_close(*(sqlite3 **)customQuery.stmt);
          nt2custom.erase(ntStmt);
        }
        return SQLITE_DONE;
      }
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
      printf("[%s]SQLITE_ROW return\n", u);
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
        if (sql.find("SELECT * FROM group_msg_table") != std::string::npos) {
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
              
              auto newRow = conver2NTMem((Vdbe *)newStmt, ntVdbe);

              // printf("read data\n");
              // for (int i=0; i < ntVdbe->nResColumn; i++) {
              //   auto cur = newRow[i];
              //   if (cur.flags & MEM_Str) {
              //     printf("MEM_Str[%d]: %s\n", cur.n, cur.z);
              //   }
              //   else if(cur.flags & MEM_Int) {
              //     printf("MEM_Int[%ld]\n", cur.u.i);
              //   }
              //   else if (cur.flags & MEM_Blob) {
              //     printf("MEM_Blob[%d]\n", cur.n);
              //   }
              // }

              printf("[%s]end!!!\n", u);
              ntVdbe->pResultRow = newRow;
              // 成功
              CustomQuery q = {newStmt, newRow};
              nt2custom.emplace(std::pair<sqlite3_stmt *, CustomQuery>(ntStmt, q));
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