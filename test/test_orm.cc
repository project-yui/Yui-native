#include <iostream>
#include "../src/include/db/group_msg_table.hh"

using namespace std;
int main(void) {
  cout << "orm test" << endl;
  nt_db::GroupMsgTableDb db;
  nt_model::GroupMsgTable data;
  data.msgId = 12345670;
  data.msgSeq = 1;
  
  db.add(data);
  return 0;
}