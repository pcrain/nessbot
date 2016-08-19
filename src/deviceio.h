#ifndef DEVICEIO_H_
#define DEVICEIO_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/time.h>

#include "termoutput.h"

namespace nessbot {

enum input_name {
  inname_neutral,
  inname_left,
  inname_right,
  inname_jump,
  // inname_recover,
  // inname_crouch,
  // inname_smash_left,
  // inname_smash_right,
  // inname_smash_up,
  // inname_smash_down,
  inname_end
};

const std::string inname_string[inname_end+1] = {
  "neutral     ",
  "left        ",
  "right       ",
  "jump        ",
  // "recover     ",
  // "crouch      ",
  // "smash_left  ",
  // "smash_right ",
  // "smash_up    ",
  // "smash_down  ",
  "end"
};

struct input_event;
struct input_combo;

int init_device();
int device_readloop();
int device_test();
void device_write(input_event ie, bool sync = false);
void device_write(input_combo ic);
void device_sync();
int close_device();
void act(input_name in);

}

#endif /* DEVICEIO_H_ */
