#include <iostream>
#include <fstream>
#include "../src/proto/message.pb.h"

int main(void) {
  nt_msg::Elements elems;
  auto elem = elems.add_elem();
  elem->set_msgtype(nt_msg::Element_MsgType_MSG_TYPE_TEXT);
  elem->set_msgid(7339004109022003740);
  elem->set_textstr("77677677677677677");
  elem->set_attype(0);
  std::fstream output("./output.bin", std::ios::out | std::ios::trunc | std::ios::binary);
  elems.SerializeToOstream(&output);
  // std::cout << output.c_str() << std::endl;
  return 0;
}