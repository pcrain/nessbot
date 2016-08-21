#ifndef MEMREADER_H_
#define MEMREADER_H_

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <unistd.h>

#include <vector>
#include <map>
#include <string>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>

#include "termoutput.h"
#include "util.h"

namespace nessbot {

  const unsigned long RAMOFFSET = std::stoul("2500000000", nullptr, 16);
  const unsigned long GTADDRESS = RAMOFFSET + std::stoul("80479D60", nullptr, 16);

  struct ram_address_info_raw {
    std::string name;
    std::string vtype;
    std::vector<std::string> address_strings;
  };

  struct ram_address_info {
    std::string name;
    std::string vtype;
    unsigned long address;
  };

  union ram_value {
    float f;
    unsigned long u;
    signed long s;
  };

  extern unsigned long p1_state_address;
  extern std::vector<struct ram_address_info_raw> raw_addresses;

  extern std::vector<struct ram_address_info> byte_offsets;
  extern std::map<std::string, unsigned> named_byte_map;

  int init_memreader();
  int close_memreader();
  void precompute_offsets();
  std::vector<ram_value> get_game_state();
  void monitor_game_state();
  ram_value get_game_byte(unsigned long address, int& errorflag);
}

#endif /* MEMREADER_H_ */
