#include "memreader.h"

namespace nessbot {

unsigned int dolphin_pid;
FILE* _dolphin_memory_descriptor;
std::vector<struct ram_address_info> byte_offsets;
unsigned long p1_state_address = 0;
std::string dolphin_memfile;

int init_memreader() {
  char buf[512];
  FILE *cmd_pipe = popen("pgrep dolphin", "r");
  fgets(buf, 512, cmd_pipe);
  dolphin_pid = strtoul(buf, NULL, 10);
  pclose( cmd_pipe );

  // curprint("%u\n",dolphin_pid); refresh();
  if (dolphin_pid == 0) {
    return -1; //Dolphin not found
  }

  dolphin_memfile = ("/proc/"+std::to_string(dolphin_pid)+"/mem");
  _dolphin_memory_descriptor = fopen(dolphin_memfile.c_str(), "rb");
  // curprint("%u\n",_dolphin_memory_descriptor); refresh();
  if (_dolphin_memory_descriptor == 0) {
    return -2; //Not running as root
  }

  std::vector<struct ram_address_info_raw> raw_addresses = {
    {"P1 X    ","float",{PLAYERENTITY[0] + std::stoul("B0",nullptr,16), hexint("2C"), hexint("B0")}},
    {"P1 Y    ","float",{PLAYERENTITY[0] + std::stoul("B0",nullptr,16), hexint("2C"), hexint("B4")}},
    {"P1 State","int",  {PLAYERENTITY[0] + std::stoul("B0",nullptr,16), hexint("2C"), hexint("10")}},
    {"P2 X    ","float",{PLAYERENTITY[1] + std::stoul("B0",nullptr,16), hexint("2C"), hexint("B0")}},
    {"P2 Y    ","float",{PLAYERENTITY[1] + std::stoul("B0",nullptr,16), hexint("2C"), hexint("B4")}},
    {"P2 State","int",  {PLAYERENTITY[1] + std::stoul("B0",nullptr,16), hexint("2C"), hexint("10")}},
    {"P1 VX   ","float",{PLAYERENTITY[0] + std::stoul("B0",nullptr,16), hexint("2C"), hexint("C8")}},
    {"P1 VY   ","float",{PLAYERENTITY[0] + std::stoul("B0",nullptr,16), hexint("2C"), hexint("CC")}}
  };

  byte_offsets = precompute_offsets(raw_addresses);

  return 0;
}

int close_memreader() {
  if (_dolphin_memory_descriptor != nullptr) {
    fclose (_dolphin_memory_descriptor);
  }
  return 0;
}

void monitor_game_state() {
  std::vector<unsigned long> gs;
  while (true) {
    gs = get_game_state();
    curclear();
    for (unsigned i = 0; i < gs.size(); ++i) {
      if (byte_offsets[i].vtype == "float") {
        curprint("%u: %f\n",i,hexfloat(gs[i]));
      }
      else if (byte_offsets[i].vtype == "int") {
        curprint("%u: %lu\n",i,(unsigned long)gs[i]);
      }
    }
    refresh();
    fsleep(1);
  }
}

std::vector<struct ram_address_info> precompute_offsets(std::vector<struct ram_address_info_raw> raw_addresses) {
  std::vector<struct ram_address_info> computed_address;

  for (unsigned i = 0; i < raw_addresses.size(); ++i) {
    bool first = true;
    unsigned long start = RAMOFFSET;
    signed long chunk = 0;
    char ramdata[CHUNKSIZE];
    // curprint("OFFSET: %lu\n",start); refresh();
    for (unsigned j = 0; j < raw_addresses[i].address_strings.size(); ++j) {
      if (!first) {
        start = RAMOFFSET + chunk;
      }
      start += raw_addresses[i].address_strings[j];
      if (start < RAMOFFSET) {
        start = RAMOFFSET; //Dummy value for invalid memory address
        break;
      }
      fseek(_dolphin_memory_descriptor,start,SEEK_SET);
      fgets(ramdata,CHUNKSIZE+1,_dolphin_memory_descriptor);
      endianfix(ramdata);
      memcpy(&chunk, &ramdata, CHUNKSIZE);
      // curprint("Chunk %u,%u: %li -> %li\n",i,j,start,chunk); refresh();
      first = false;
    }
    if (raw_addresses[i].name.compare("P1 State") == 0) {
      p1_state_address = start;
    }
    computed_address.push_back({raw_addresses[i].name,raw_addresses[i].vtype,start});
  }
  // fsleep(6000);
  return computed_address;
}

std::vector<unsigned long> get_game_state() {
  std::vector<unsigned long> gs;

  if (! file_available(dolphin_memfile.c_str())) {
    return gs;
  }
  unsigned long chunk;
  char ramdata[CHUNKSIZE];
  for (unsigned i = 0; i < byte_offsets.size(); ++i) {
    fseek(_dolphin_memory_descriptor,byte_offsets[i].address,SEEK_SET);
    fgets(ramdata,CHUNKSIZE+1,_dolphin_memory_descriptor);
    endianfix(ramdata);
    memcpy(&chunk, &ramdata, CHUNKSIZE);
    gs.push_back(chunk);
  }

  return gs;
}

unsigned long get_game_byte(unsigned long address, int& errorflag) {
  unsigned long chunk = 0;
  char ramdata[CHUNKSIZE];

  if (! file_available(dolphin_memfile.c_str())) {
    errorflag = -1;
    return 0;
  }

  fseek(_dolphin_memory_descriptor,0,SEEK_SET);  //Reset seek pointer
  fseek(_dolphin_memory_descriptor,address,SEEK_SET);
  fgets(ramdata,CHUNKSIZE+1,_dolphin_memory_descriptor);
  endianfix(ramdata);
  memcpy(&chunk, &ramdata, CHUNKSIZE);

  return chunk;
}

}
