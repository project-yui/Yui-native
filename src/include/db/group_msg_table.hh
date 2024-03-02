#ifndef __DB_GROUP_MSG_TABLE_HH__
#define __DB_GROUP_MSG_TABLE_HH__
#include "sqlite_orm/sqlite_orm.h"
#include "model/group_msg_table.hh"
#include "../handle.hh"
#include <cstdio>


namespace nt_db {
  static auto storage = sqlite_orm::make_storage(yukihana::db_name,
                              sqlite_orm::make_table("group_msg_table",
                                        sqlite_orm::make_column("40001", &nt_model::GroupMsgTable::msgId, sqlite_orm::primary_key()),
                                        sqlite_orm::make_column("40002", &nt_model::GroupMsgTable::msgRandom),
                                        sqlite_orm::make_column("40003", &nt_model::GroupMsgTable::msgSeq),
                                        sqlite_orm::make_column("40010", &nt_model::GroupMsgTable::chatType),
                                        sqlite_orm::make_column("40011", &nt_model::GroupMsgTable::msgType),
                                        sqlite_orm::make_column("40012", &nt_model::GroupMsgTable::subMsgType),
                                        sqlite_orm::make_column("40013", &nt_model::GroupMsgTable::sendType),
                                        sqlite_orm::make_column("40020", &nt_model::GroupMsgTable::senderUid),
                                        sqlite_orm::make_column("40021", &nt_model::GroupMsgTable::peerUid),
                                        sqlite_orm::make_column("40027", &nt_model::GroupMsgTable::peerUidLong),
                                        sqlite_orm::make_column("40050", &nt_model::GroupMsgTable::msgTime),
                                        sqlite_orm::make_column("40041", &nt_model::GroupMsgTable::sendStatus),
                                        sqlite_orm::make_column("40090", &nt_model::GroupMsgTable::sendMemberName),
                                        sqlite_orm::make_column("40093", &nt_model::GroupMsgTable::sendNickName),
                                        sqlite_orm::make_column("40800", &nt_model::GroupMsgTable::elements),
                                        sqlite_orm::make_column("40033", &nt_model::GroupMsgTable::senderUin),
                                        sqlite_orm::make_column("40005", &nt_model::GroupMsgTable::clientSeq),
                                        sqlite_orm::make_column("40100", &nt_model::GroupMsgTable::atType)
                                        ));
  
  /**
   * @brief CRUD基础操作
   * 
   * @tparam T 
   */
  template <typename T>
  class NTDb {
    public:
    NTDb(){
      printf("NTDb constructor\n");
      printf("db file: %s\n", yukihana::db_name);
      storage.sync_schema(true);
    };
    virtual bool add(T &data) = 0;
  };
  
  /**
   * @brief CURD具体实现
   * 
   */
  class GroupMsgTableDb: public NTDb<nt_model::GroupMsgTable> {
    public:
    bool add(nt_model::GroupMsgTable& data) override;
    private:
      
  };
}
#endif