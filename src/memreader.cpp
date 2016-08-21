#include "memreader.h"

namespace nessbot {

unsigned int dolphin_pid;
FILE* _dolphin_memory_descriptor;
std::vector<struct ram_address_info> byte_offsets;
std::map<std::string, unsigned> named_byte_map;
unsigned long p1_state_address = 0;
std::string dolphin_memfile;

//Can add 0xB0 to get player entity data
//(see raw_addresses below)
const unsigned long PLAYERENTITY[4] = {
  hexint("80453080"),
  hexint("80453F10"),
  hexint("80454DA0"),
  hexint("80455C30")
};

//Backup for if config file fails to load
std::vector<struct ram_address_info_raw> raw_addresses = {
  {"P1 X","float",{"80453130", "2C", "B0"}},
  {"P1 Y","float",{"80453130", "2C", "B4"}},
  {"P1 State","int",  {"80453130", "2C", "10"}},
  {"P2 X","float",{"80453fc0", "2C", "B0"}},
  {"P2 Y","float",{"80453fc0", "2C", "B4"}},
  {"P2 State","int",  {"80453F10", "2C", "10"}},
  {"P1 VX","float",{"80453130", "2C", "C8"}},
  {"P1 VY","float",{"80453130", "2C", "CC"}}
};

//Alternate backup
// std::vector<struct ram_address_info_raw> raw_addresses = {
//   {"Dummy   ","float",{"00000000"}}
// };

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

  precompute_offsets();

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

void precompute_offsets() {
  std::vector<struct ram_address_info> computed_address;
  std::map<std::string, unsigned> named_bytes;

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
      start += hexint(raw_addresses[i].address_strings[j]);
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
    named_bytes[raw_addresses[i].name] = i;
  }
  // fsleep(6000);
  byte_offsets = computed_address;
  named_byte_map = named_bytes;
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
