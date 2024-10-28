#ifndef __DB_GROUP_MSG_TABLE_HH__
#define __DB_GROUP_MSG_TABLE_HH__
#include "sqlite_orm/sqlite_orm.h"
#include "model/group_msg_table.hh"
#include "../handle.hh"
#include <spdlog/spdlog.h>


namespace nt_db {
  static auto storage = sqlite_orm::make_storage(yui::db_name,
                              sqlite_orm::make_table("group_msg_table",
                                        sqlite_orm::make_column("40001", &nt_model::GroupMsgTable::msgId, sqlite_orm::primary_key()),
                                        sqlite_orm::make_column("40002", &nt_model::GroupMsgTable::msgRandom),
                                        sqlite_orm::make_column("40003", &nt_model::GroupMsgTable::msgSeq),
                                        sqlite_orm::make_column("40010", &nt_model::GroupMsgTable::chatType),
                                        sqlite_orm::make_column("40011", &nt_model::GroupMsgTable::msgType),
                                        sqlite_orm::make_column("40012", &nt_model::GroupMsgTable::subMsgType),
                                        sqlite_orm::make_column("40013", &nt_model::GroupMsgTable::sendType),
                                        sqlite_orm::make_column("40020", &nt_model::GroupMsgTable::senderUid),
                                        // 1,2,3 权限 1:群主 2:管理员 3:群成员
                                        sqlite_orm::make_column("40026", &nt_model::GroupMsgTable::userLevel),
                                        sqlite_orm::make_column("40021", &nt_model::GroupMsgTable::peerUid),
                                        sqlite_orm::make_column("40027", &nt_model::GroupMsgTable::peerUidLong),
                                        // 0或1
                                        // sqlite_orm::make_column("40040", &nt_model::GroupMsgTable::peerUidLong),
                                        sqlite_orm::make_column("40041", &nt_model::GroupMsgTable::sendStatus),
                                        sqlite_orm::make_column("40050", &nt_model::GroupMsgTable::msgTime),
                                        // 0
                                        // sqlite_orm::make_column("40052", &nt_model::GroupMsgTable::msgTime),
                                        sqlite_orm::make_column("40090", &nt_model::GroupMsgTable::sendMemberName),
                                        sqlite_orm::make_column("40093", &nt_model::GroupMsgTable::sendNickName),
                                        sqlite_orm::make_column("40800", &nt_model::GroupMsgTable::elements),
                                        // null或blob
                                        // sqlite_orm::make_column("40900", &nt_model::GroupMsgTable::elements),
                                        // 0或1 editable
                                        sqlite_orm::make_column("40105", &nt_model::GroupMsgTable::editable),
                                        sqlite_orm::make_column("40005", &nt_model::GroupMsgTable::clientSeq),
                                        // 此消息msgTime当天的00:00
                                        sqlite_orm::make_column("40058", &nt_model::GroupMsgTable::todayZero),
                                        // 0 或 一长串数字
                                        // sqlite_orm::make_column("40006", &nt_model::GroupMsgTable::clientSeq),
                                        sqlite_orm::make_column("40100", &nt_model::GroupMsgTable::atType),
                                        // blob
                                        // sqlite_orm::make_column("40600", &nt_model::GroupMsgTable::atType),
                                        // 0
                                        // sqlite_orm::make_column("40060", &nt_model::GroupMsgTable::atType),
                                        // replyMsgSeq 引用回复的目标msgSeq
                                        sqlite_orm::make_column("40850", &nt_model::GroupMsgTable::replyMsgSeq),
                                        // 0
                                        // sqlite_orm::make_column("40851", &nt_model::GroupMsgTable::atType),
                                        // null
                                        // sqlite_orm::make_column("40601", &nt_model::GroupMsgTable::atType),
                                        // blob
                                        // sqlite_orm::make_column("40801", &nt_model::GroupMsgTable::atType),
                                        // null
                                        // sqlite_orm::make_column("40605", &nt_model::GroupMsgTable::atType),
                                        sqlite_orm::make_column("40030", &nt_model::GroupMsgTable::peerUidLong),
                                        sqlite_orm::make_column("40033", &nt_model::GroupMsgTable::senderUin)
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
      spdlog::debug("NTDB sync_schema start");
      storage.sync_schema();
      spdlog::debug("NTDB sync_schema end");
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