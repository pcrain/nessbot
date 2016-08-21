#include "deviceio.h"

namespace nessbot {

struct input_event {
  struct timeval time;
  unsigned short type;
  unsigned short code;
  unsigned int value;
}; //Size == 24 bytes

struct input_combo {
  unsigned char x;
  unsigned char y;
  unsigned char n;
  unsigned char e;
  unsigned char w;
  unsigned char s;
};

const char* EVDEVICE = "/dev/input/event14";

const int IE_SIZE = sizeof(input_event);
const int IC_SIZE = sizeof(input_event);

const struct input_event SYNC_EVENT = {{0,0},EV_SYN,SYN_REPORT,0};

const struct input_combo input_neutral       = {136,136,0,0,0,0};
const struct input_combo input_left          = {20, 136,0,0,0,0};
const struct input_combo input_right         = {235,136,0,0,0,0};
const struct input_combo input_jump          = {136,136,1,0,0,0};
const struct input_combo input_recover       = {136,20 ,0,0,1,0};
const struct input_combo input_crouch        = {136,235,0,0,0,0};
const struct input_combo input_smash_left    = {20, 136,0,0,0,1};
const struct input_combo input_smash_right   = {235,136,0,0,0,1};
const struct input_combo input_smash_up      = {136,20, 0,0,0,1};
const struct input_combo input_smash_down    = {136,235,0,0,0,1};

static int _devince_input_descriptor = -1;

int init_device() {
  _devince_input_descriptor = open(EVDEVICE, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
  if (_devince_input_descriptor == -1) {
    // perror("open_port: Unable to open /dev/input/event14 - ");
    close(_devince_input_descriptor);
    return(-1);
  }
  return 0;
}

int device_readloop() {
  int n;
  char buf[256];

  while (true) {
    curclear();
    n = read(_devince_input_descriptor, (void*)buf, 255);

    for (int i = 0; i < n; i += IE_SIZE) {
      input_event ie;
      memcpy(&ie, &(buf[i]), IE_SIZE);

      curprint("Input event:\n");
      curprint("  type = %u,%u,%u:\n",ie.type,ie.code,ie.value);
      curprint("%i\n",EV_KEY);
      fsleep(1);
    }
  }
}

int device_test() {
  // device_readloop();

  input_event ie;
  memset(&ie, 0, IE_SIZE);
  ie.type = EV_ABS;
  ie.code = ABS_X;
  for (int i = 0; i < 500; ++ i) {
    ie.value = 20;
    device_write(input_left);
    fsleep(4);

    ie.value = 235;
    device_write(input_right);
    fsleep(4);
  }
  ie.value = 130;
  device_write(ie);

  curreset();

  return 0;
}

int close_device() {
  close(_devince_input_descriptor);
  return 0;
}

void device_write(input_event ie, bool sync) {
  write(_devince_input_descriptor, &ie, IE_SIZE);
  if (sync)
    write(_devince_input_descriptor, &SYNC_EVENT, IE_SIZE);
}

int act(input_name in) {
  if (! file_available(EVDEVICE)) {
    return -1;
  }
  switch(in) {
    case inname_neutral     : device_write(input_neutral);     break;
    case inname_left        : device_write(input_left);        break;
    case inname_right       : device_write(input_right);       break;
    case inname_jump        : device_write(input_jump);        break;
    case inname_recover     : device_write(input_recover);     break;
    case inname_crouch      : device_write(input_crouch);      break;
    case inname_smash_left  : device_write(input_smash_left);  break;
    case inname_smash_right : device_write(input_smash_right); break;
    case inname_smash_up    : device_write(input_smash_up);    break;
    case inname_smash_down  : device_write(input_smash_down);  break;
    default                 : device_write(input_neutral);     break;
  }
  return 0;
}

void device_write(input_combo ic){
  // curprint("WRITE %u,%u,%u,%u,%u,%u\n",ic.x,ic.y,ic.n,ic.e,ic.w,ic.s);
  input_event ie;
  memset(&ie, 0, IE_SIZE);
  ie.type = EV_ABS;
  ie.code = ABS_X;
  ie.value = ic.x;
  write(_devince_input_descriptor, &ie, IE_SIZE);
  ie.code = ABS_Y;
  ie.value = ic.y;
  write(_devince_input_descriptor, &ie, IE_SIZE);
  ie.type = EV_KEY;
  ie.code = BTN_NORTH;
  ie.value = ic.n;
  write(_devince_input_descriptor, &ie, IE_SIZE);
  ie.code = BTN_EAST;
  ie.value = ic.e;
  write(_devince_input_descriptor, &ie, IE_SIZE);
  ie.code = BTN_WEST;
  ie.value = ic.w;
  write(_devince_input_descriptor, &ie, IE_SIZE);
  ie.code = BTN_SOUTH;
  ie.value = ic.s;
  write(_devince_input_descriptor, &ie, IE_SIZE);
  write(_devince_input_descriptor, &SYNC_EVENT, IE_SIZE);
}

void device_sync() {
  write(_devince_input_descriptor, &SYNC_EVENT, IE_SIZE);
}

}
