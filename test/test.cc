
#include <cstdint>
#include <iostream>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

int main(int argc, char const *argv[])
{
  std::set<std::string> list;
  list.emplace("1");
  list.emplace("2");
  list.emplace("3");
  list.emplace("4");
  list.emplace("5");
  list.emplace("6");
  
  std::cout << "size:" << list.size() << std::endl;

  void ** ptr = (void**)&list;
  void * start = *ptr;
  void * end = *(ptr + 1);
  int len = sizeof(std::pair<std::string, int>);
  std::cout << "size:" << ((int64_t)end - (int64_t)start) / 40 << std::endl;
  return 0;
}
