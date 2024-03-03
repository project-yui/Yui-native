#ifndef __MODEL_GROUP_MSG_TABLE_HH__
#define __MODEL_GROUP_MSG_TABLE_HH__

#include <memory>
#include <vector>
#include <string>

/**
 * @brief 数据库模型
 * 
 */
namespace nt_model {

  struct GroupMsgTable {
    /**
     * @brief 消息id
     * 40001
     * 
     */
    long msgId;

    /**
     * @brief 
     * 40002
     * 
     */
    long msgRandom;

    /**
     * @brief 
     * 40003
     * 
     */
    int msgSeq;

    /**
     * @brief 聊天类型
     * 40010
     * 
     */
    int chatType;

    /**
     * @brief 消息类型
     * 40011
     * 
     */
    int msgType;

    /**
     * @brief 
     * 40012
     * 
     */
    int subMsgType;

    /**
     * @brief 
     * 0 - 别人发的
     * 2 - 自己发的
     * 
     * 40013
     * 
     */
    int sendType;

    /**
     * @brief 发送者uid
     * 40020
     * u_xxx
     * 
     */
    std::string senderUid;

    /**
     * @brief 群号
     * 40021(text)
     * 
     * peerUin也用这个字段
     * 
     */
    std::string peerUid;

    /**
     * @brief 群号
     * 40027(integer)
     * 
     * peerUin也用这个字段
     * 
     */
    long peerUidLong;

    /**
     * @brief 消息发送时间
     * 40050
     * 
     */
    long msgTime;
    
    /**
     * @brief 
     * 40041
     * 
     */
    int sendStatus;
    // std::shared_ptr<std::string> sendRemarkName;

    /**
     * @brief 发送者群昵称
     * 40090
     * 
     */
    std::shared_ptr<std::string> sendMemberName;

    /**
     * @brief 发送者QQ昵称
     * 40093
     * 
     */
    std::shared_ptr<std::string> sendNickName;

    /**
     * @brief 消息内容
     * 40800
     * 
     */
    std::vector<char> elements;

    /**
     * @brief 群名称
     * 
     */
    std::shared_ptr<std::string> peerName;

    /**
     * @brief 发送者
     * 40033
     * 
     */
    long senderUin;
    
    /**
     * @brief 
     * 40005
     * 
     */
    int clientSeq;

    /**
     * @brief 
     * 40100
     * 0 - 无At
     * 2 - 引用回复默认At
     * 
     */
    int atType = 0;

    /**
     * @brief 
     * 40851
     * 
     */
    int replyMsgSeq = 0;

    /**
     * @brief 本消息当天00:00
     * 40058
     * 
     */
    long todayZero;

    /**
     * @brief 是否可编辑
     * 40105
     * 
     */
    int editable = 0;

    /**
     * @brief 权限 1:群主 2:管理员 3:群成员
     * 40026
     * 
     */
    int userLevel;
  };
}
#endif