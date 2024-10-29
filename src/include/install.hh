#ifndef __INSTALL_HH__
#define __INSTALL_HH__

#include <string>
#include <vector>
bool install_hosts_hook(std::string &name, std::vector<uint8_t> & feature_code);
bool install_sqlite3_hook(std::string &name, std::vector<uint8_t> & feature_code);
bool install_msf_hook(std::string &name, std::vector<uint8_t> & feature_code);
bool install_msf_response_hook(std::string &name, std::vector<uint8_t> & feature_code);

#endif