#include "../include/db/group_msg_table.hh"
#include <cstdio>
#include <spdlog/spdlog.h>

/**
 * @brief 对数据库进行操作
 * 
 */
namespace nt_db {
  // template <typename T>
  // NTDb<T>::NTDb() {
  //     storage.sync_schema();
  // }
  bool GroupMsgTableDb::add(nt_model::GroupMsgTable& data) {
    storage.replace(data);
    spdlog::debug("GroupMsgTableDb add result\n");
    return true;
  }
}