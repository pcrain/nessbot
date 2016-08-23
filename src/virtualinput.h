#ifndef VIRTUALINPUT_H_
#define VIRTUALINPUT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

namespace nessbot {

static int vi_fd;
int init_virtual_device();
int close_virtual_device();

}

#endif /* VIRTUALINPUT_H_ */
