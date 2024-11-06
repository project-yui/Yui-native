#include <iostream>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>
#include "../src/proto/communication.pb.h"
#include "spdlog/spdlog.h"

int main(void) {
  // 1. parse resp
  nt::communication::TcpBase resp;
  // 2. modify msg
  resp.set_command(2418);
  resp.set_subcommand(6);
  resp.set_errorcode(114514);
  resp.set_errormsg("");
  // 3. repack

  auto respBody = new nt::communication::StrangerSearchResp();
  auto f1 = new nt::communication::StrangerSearchRespField1();
  f1->set_field1(1001);
  f1->set_field2("查找人");
  f1->set_field4(0);
  respBody->set_allocated_field1(f1);
  respBody->set_field2(0);
  uint8_t f3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  respBody->set_field3(f3, 15);
  resp.set_body(respBody->SerializePartialAsString());

  auto size = resp.ByteSizeLong();
  uint8_t* result = new uint8_t[size];
  resp.SerializePartialToArray(result, size);

  
  std::stringstream ss;
  for (uint8_t *i = result; i < result + size; i++) {
    ss << " 0x" << std::uppercase << std::setfill('0') << std::setw(2) <<  std::hex << static_cast<unsigned int>(*i);
  }
  ss << std::endl;
  spdlog::info("data: {}", ss.str());
  delete [] result;
  return 0;
}