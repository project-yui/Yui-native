#include <cstdlib>
#include <iostream>
#include <sqlite3.h>
#include "../src/include/sqlite3/nt/vdbe.hh"
#include "../src/include/sqlite3/nt/sqlite3.hh"

void printValue(const char* name, unsigned long v) {
  printf("[%s] %ld, %x\n", name, v, v);
}

auto main(int argc, char const *argv[]) -> int
{
  std::cout << "test sqlite3" << std::endl;
  int rc = 0;
  rc = sqlite3_initialize(); // Initializes the library. If the library has already been initialized it has no effect.
  if (rc != SQLITE_OK)
  {
    std::cout << "failed to init sqlite3!" << std::endl;
    return 0;
  }
  

  sqlite3 *db = nullptr;
  const char *vfs = nullptr;
  rc = sqlite3_open_v2("./test/test.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, vfs);
  if (rc != SQLITE_OK)
  {
    sqlite3_close(db);
    std::cout << "failed to open db!" << std::endl;
    return 1;
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
    return 0;
  }

  auto querySql = sqlite3_sql(stmt);
  std::cout << "sql:" << querySql << std::endl;
  // Vdbe * info = (Vdbe *)stmt;
  
  // Now we retrieve the row
  rc = sqlite3_step(stmt);
  rc = sqlite3_step(stmt);
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW)
  {
    // Here we get a pointer to the location text ( stored in the second column of the table )
    // The 1 in sqlite3_column_text( stmt, 1 ) is the column number (zero based).
    // sqlite3_column_text( sqlite_stmt* stmt, int cidx ) returns const unsigned char* so the casts are necessary.
    void *p = const_cast<unsigned char *>(sqlite3_column_text(stmt, 1));
    const char *ret = static_cast<const char *>(p);
    std::cout << "data:" << ret << std::endl;
    NTVdbe * v = (NTVdbe *)stmt;
    int colCount = v->nResColumn;
    auto result = v->pResultRow;
    for (int i=0; i < colCount; i++) {
      auto data = result[i];

      printf("col[%d] data\n", i);

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

  }
  else
  {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return 0;
}
