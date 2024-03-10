
#include <cstdlib>
#include <memory>
#include <spdlog/spdlog.h>
#include <map>
#include <sqlite3.h>
#include <string>
#include <utility>
#include "../include/handle.hh"
#include "../include/sqlite3/nt/sqlite3.hh"
#include "../include/sqlite3/nt/base.hh"
#include "../include/sqlite3/official/base.hh"
#include "../include/sqlite3/official/sqlite3.hh"
#include "../include/sqlite3/nt/vdbe.hh"
#include "../include/sqlite3/official/vdbe.hh"

namespace yukihana {
  const char* db_name = "./yukihana.db";
  struct CustomQuery {
    sqlite3_stmt * stmt;
    NTMem * row;
  };
  std::shared_ptr<NTNative::Hook> hook;
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
      int srcColN = sqlite3_column_count((sqlite3_stmt *)src);
    spdlog::debug("conver2NTMem: {}", srcColN);
    // sleep(2);
    // return rowTest;
    std::map<std::string, Mem*> srcData;
    for (int i=0; i < srcColN; i++) {
      auto col = src->aColName[i];
      spdlog::debug("column name length: {}", col.n);
      spdlog::debug("column name: {}", col.z);
      auto data = src->pResultRow +i;
      std::string name(col.z);
      srcData.emplace(std::pair<std::string, Mem *>(name, data));
    }
    
    spdlog::debug("malloc column: {}", dest->nResColumn);
    NTMem * result = (NTMem *)malloc(sizeof(NTMem) * dest->nResColumn);

    spdlog::debug("start copy data");
    for (int i=0; i < dest->nResColumn; i++) {
      auto col = dest->aColName[i];
      spdlog::debug("[{}]copy data: {}", col.z, i);
      std::string name(col.z);
      NTMem* target = result + i;
      
      target->u.i = 0;
      target->flags = MEM_Null;
      target->enc = 0;
      target->eSubtype = 0;
      target->n = 0;
      target->z = nullptr;
      target->zMalloc = nullptr;

      if (srcData.find(name) == srcData.end()) {
        // if (name == "40600") {
        //   printf("special 40600");
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 8;
        //   target->z = (char *)data_2;
        //   target->zMalloc = (char *)data_2;
        //   continue;
        // }
        // else if(name == "40801") {
        //   printf("special 40801");
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 78;
        //   target->z = (char *)data_3;
        //   target->zMalloc = (char *)data_3;
        //   continue;
        // }
        // //
        // else if(name == "40040") {
        //   printf("special 40040");
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
        //   printf("special 40052");
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
        //   printf("special 40900");
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 0;
        //   target->z = "";
        //   target->zMalloc = "";
        //   continue;
        // }
        // else if(name == "40006") {
        //   printf("special 40006");
        //   target->u.i = 0;
        //   target->flags = MEM_Int;
        //   target->enc = 0;
        //   target->n = 0;
        //   target->z = nullptr;
        //   target->zMalloc = nullptr;
        //   continue;
        // }
        // else if(name == "40060") {
        //   printf("special 40060");
        //   target->u.i = 0;
        //   target->flags = MEM_Int;
        //   target->enc = 0;
        //   target->n = 0;
        //   target->z = nullptr;
        //   target->zMalloc = nullptr;
        //   continue;
        // }
        // else if(name == "40851") {
        //   printf("special 40851");
        //   target->u.i = 0;
        //   target->flags = MEM_Int;
        //   target->enc = 0;
        //   target->n = 0;
        //   target->z = nullptr;
        //   target->zMalloc = nullptr;
        //   continue;
        // }
        // else if(name == "40601") {
        //   printf("special 40601");
        //   target->u.i = 0;
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 0;
        //   target->z = "";
        //   target->zMalloc = "";
        //   continue;
        // }
        // else if(name == "40605") {
        //   printf("special 40605");
        //   target->u.i = 0;
        //   target->flags = MEM_Blob;
        //   target->enc = SQLITE_UTF8;
        //   target->n = 0;
        //   target->z = "";
        //   target->zMalloc = "";
        //   continue;
        // }
        spdlog::debug("not found : {}", name.c_str());
        target->n = 0;
        target->flags = MEM_Null;
        target->z = target->zMalloc = nullptr;
        continue;
      }

      Mem * cur = srcData[name];
      if (cur->flags & MEM_Int) {
        // 数值
        target->flags = MEM_Int;
        target->u.i = cur->u.i;
        spdlog::debug("MEM_Int: {}", cur->u.i);
      }
      else if (cur->flags & (MEM_Str | MEM_Term)) {
        // 字符串
        target->flags = MEM_Str | MEM_Term;
        target->enc = SQLITE_UTF8;
        target->n = cur->n;
        target->z = cur->z;
        target->zMalloc = cur->zMalloc;
        spdlog::debug("MEM_Str[{}]: {}", target->n, cur->z);
      }
      else if (cur->flags & MEM_Blob) {
        // 二进制
        target->flags = MEM_Blob;
        target->z = cur->z;
        target->n = cur->n;
        target->enc = SQLITE_UTF8;
        target->zMalloc = cur->zMalloc;
        spdlog::debug("MEM_Blob[{}]", target->n);
        // *target = rowTest[i];
      }
      else {
        spdlog::debug("not supported!");
        target->z = nullptr;
        target->flags = MEM_Null;
        target->zMalloc = nullptr;
        target->n = 0;
        target->u.i = 0;
      }
    }
    return result;
  }

  void printNTVdbeInfo(NTVdbe * vdbe) {
      spdlog::debug("nt col num: {}", vdbe->nResColumn);
      for (int i = 0; i < vdbe->nResColumn; i++)
      {
          spdlog::debug("col[{}]: {}", i+1, vdbe->aColName[i].z);
          spdlog::debug("------------data---------------");
          auto cur = vdbe->pResultRow[i];

          if (cur.flags & MEM_Int) {
              // 数值
              spdlog::debug("MEM_Int: {}", cur.u.i);
          }
          else if (cur.flags & (MEM_Str | MEM_Term)) {
              // 字符串
              spdlog::debug("MEM_Str[{}]: {}", cur.n, cur.z);
          }
          else if (cur.flags & MEM_Blob) {
              // 二进制
              spdlog::debug("MEM_Blob[{}]", cur.n);
              // *target = rowTest[i];
          }
          else {
              spdlog::debug("not supported!");
          }
      }
  }
  int execute(void* a1,
  void* a2,
  void* a3,
  void* a4,
  void* a5,
  void* a6) {
    
    spdlog::debug("execute");
    stmt_func fun = (stmt_func)hook->get_trampoline();
    if (fun == nullptr) {
        spdlog::debug("error nullptr!!!");
        return -1;
    }
    sqlite3_stmt *ntStmt = (sqlite3_stmt *)a1;
    NTVdbe * ntVdbe = (NTVdbe *)ntStmt;
    
    spdlog::debug("try to find handle");
    if (ntStmt != nullptr) {
      // 替换
      if (nt2custom.find(ntStmt) != nt2custom.end()) {
        spdlog::debug("handle found!");
        auto customQuery = nt2custom[ntStmt];
        // 找到sql实例
        int rc = sqlite3_step(customQuery.stmt);
        if (rc == SQLITE_ROW) {
          spdlog::debug("continue copy row, free old row");
          free(customQuery.row);
          int colCount = sqlite3_column_count(customQuery.stmt);
          auto newRow = conver2NTMem((Vdbe *)customQuery.stmt, ntVdbe);
          ntVdbe->pResultRow = newRow;
          customQuery.row = newRow;
          nt2custom[ntStmt] = customQuery;
          spdlog::debug("return after copy row: {}", (void *)newRow);
          return SQLITE_ROW;
        }
        else {
          spdlog::debug("no row to copy, try to free old row and close db.");
          spdlog::debug("free row... {}", (void *)customQuery.row);
          if (customQuery.row != nullptr) {
            free(customQuery.row);
          }
          else {
            spdlog::warn("no row to free, nullptr!!!");
          }
          spdlog::debug("sqlite3_finalize...");
          sqlite3_finalize(customQuery.stmt);
          spdlog::debug("sqlite3_close...");
          sqlite3_close(*(sqlite3 **)customQuery.stmt);
          spdlog::debug("erase...");
          nt2custom.erase(ntStmt);
        }
        spdlog::debug("return after free memory.");
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
      spdlog::debug("SQLITE_ROW return");
      // printNTVdbeInfo(ntVdbe);
      return ret;
    }
    // 3. 非SQLITE_ROW继续
    
    spdlog::debug("stmt(...) called: {}", ret);;
    if (ntStmt != nullptr)
    {
        spdlog::debug("read from v");
        // 4. 取sql执行
        spdlog::debug("actual sql1 -> {}", ntVdbe->zSql);
        if (ntVdbe->zSql == nullptr) {
            return ret;
        }
        // 5. 有数据就替换stmt的row内容，并返回SQLITE_ROW
        std::string sql(ntVdbe->zSql);
        spdlog::debug("sql1: {}", sql.c_str());
        spdlog::debug("ret: {}", ret);
        if (sql.find("SELECT * FROM group_msg_table") != std::string::npos) {
            spdlog::debug("try to read record from custom db!");
            sqlite3 *db = nullptr;
            const char *vfs = nullptr;
            int rc = 0;
            spdlog::debug("try to open custom db!");
            rc = sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, vfs);
            if (rc != SQLITE_OK)
            {
                sqlite3_close(db);
                spdlog::debug("failed to open db!");
                return ret;
            }

            // Now we create an SQL command which is stored in an sqlite3_stmt data structure.
            // Note symColName_ is a member of EquityDataLocator
            sqlite3_stmt *newStmt = nullptr;
            spdlog::debug("try to prepare sql[{}]: {}", strlen(ntVdbe->zSql), ntVdbe->zSql);
            rc = sqlite3_prepare_v2(db, ntVdbe->zSql, strlen(ntVdbe->zSql), &newStmt, nullptr);
            if (rc != SQLITE_OK)
            {
                sqlite3_finalize(newStmt);
                sqlite3_close(db);
                spdlog::debug("failed to prepare sql!");
                return ret;
            }
            auto querySql = sqlite3_sql(newStmt);
            spdlog::debug("sql: {}", querySql);
            // Vdbe * info = (Vdbe *)stmt;
            
            // Now we retrieve the row
            rc = sqlite3_step(newStmt);
            if (rc == SQLITE_ROW)
            {
              spdlog::debug("has external data!!");
              // Here we get a pointer to the location text ( stored in the second column of the table )
              // The 1 in sqlite3_column_text( stmt, 1 ) is the column number (zero based).
              // sqlite3_column_text( sqlite_stmt* stmt, int cidx ) returns const unsigned char* so the casts are necessary.
              
              auto newRow = conver2NTMem((Vdbe *)newStmt, ntVdbe);

              // printf("read data");
              // for (int i=0; i < ntVdbe->nResColumn; i++) {
              //   auto cur = newRow[i];
              //   if (cur.flags & MEM_Str) {
              //     printf("MEM_Str[%d]: %s", cur.n, cur.z);
              //   }
              //   else if(cur.flags & MEM_Int) {
              //     printf("MEM_Int[%ld]", cur.u.i);
              //   }
              //   else if (cur.flags & MEM_Blob) {
              //     printf("MEM_Blob[%d]", cur.n);
              //   }
              // }

              spdlog::debug("end!!!");
              ntVdbe->pResultRow = newRow;
              // 成功
              CustomQuery q = {newStmt, newRow};
              nt2custom.emplace(std::pair<sqlite3_stmt *, CustomQuery>(ntStmt, q));
              return rc;
            }
            else
            {
              spdlog::debug("close db...");
              sqlite3_finalize(newStmt);
              sqlite3_close(db);
              return rc;
            }
        }
        // sqlite3_finalize(stmt);
        // sqlite3_close(db);
        // 6. 没数据就返回原值
    }
    
    spdlog::debug("result: {}", ret);
    return SQLITE_DONE;

  }
}