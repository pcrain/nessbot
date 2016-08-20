#ifndef MEMREADER_H_
#define MEMREADER_H_

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <unistd.h>

#include <vector>
#include <string>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>

#include "termoutput.h"
#include "util.h"

namespace nessbot {

  extern unsigned long p1_state_address;

  const unsigned long RAMOFFSET = std::stoul("2500000000", nullptr, 16);
  const unsigned long GTADDRESS = RAMOFFSET + std::stoul("80479D60", nullptr, 16);

  const unsigned long PLAYERENTITY[4] = {
    std::stoul("80453080", nullptr, 16),
    std::stoul("80453F10", nullptr, 16),
    std::stoul("80454DA0", nullptr, 16),
    std::stoul("80455C30", nullptr, 16)
  };

  struct ram_address_info_raw {
    std::string name;
    std::string vtype;
    std::vector<unsigned long> address_strings;
  };

  struct ram_address_info {
    std::string name;
    std::string vtype;
    unsigned long address;
  };

  int init_memreader();
  int close_memreader();
  std::vector<struct ram_address_info> precompute_offsets(std::vector<struct ram_address_info_raw> raw_addresses);
  std::vector<unsigned long> get_game_state();
  void monitor_game_state();
  unsigned long get_game_byte(unsigned long address, int& errorflag);
}

#endif /* MEMREADER_H_ */
