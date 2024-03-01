#ifndef __MESSAGE_HH__
#define __MESSAGE_HH__

#include "element.hh"
#include "string"
#include "vector"

struct Message {
  long msgId;
  long msgRandom;
  int msgSeq;
  int cntSeq;
  int chatType;
  int msgType;
  int subMsgType;
  int sendType;
  std::string senderUid;
  std::string peerUid;
  std::string channelId;
  std::string guildId;
  std::string guildCode;
  long msgTime;
  std::string msgMeta = "0x";
  int sendStatus;
  std::string sendNickName;
  std::string peerName;
  std::string senderUin;
  std::string peerUin;
  std::vector<Element> elements;
};

#endif