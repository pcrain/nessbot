#include "virtualinput.h"

namespace nessbot {

int init_virtual_device() {
  struct uinput_user_dev uidev;

  vi_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if(vi_fd < 0) {
    return -2;
  }

  if (ioctl(vi_fd, UI_SET_EVBIT, EV_KEY) < 0) {
    return -1;
  }
  if (ioctl(vi_fd, UI_SET_KEYBIT, BTN_NORTH) < 0) {
    return -1;
  }
  if (ioctl(vi_fd, UI_SET_KEYBIT, BTN_EAST) < 0) {
    return -1;
  }
  if (ioctl(vi_fd, UI_SET_KEYBIT, BTN_WEST) < 0) {
    return -1;
  }
  if (ioctl(vi_fd, UI_SET_KEYBIT, BTN_SOUTH) < 0) {
    return -1;
  }
  if (ioctl(vi_fd, UI_SET_EVBIT, EV_ABS) < 0) {
    return -1;
  }
  if (ioctl(vi_fd, UI_SET_ABSBIT, ABS_X) < 0) {
    return -1;
  }
  if (ioctl(vi_fd, UI_SET_ABSBIT, ABS_Y) < 0) {
    return -1;
  }

  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "NeSSBOT-AI");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor  = 0x1;
  uidev.id.product = 0x1;
  uidev.id.version = 1;
  uidev.absmax[ABS_X] = 235;
  uidev.absmin[ABS_X] = 20;
  uidev.absmax[ABS_Y] = 235;
  uidev.absmin[ABS_Y] = 20;

  if(write(vi_fd, &uidev, sizeof(uidev)) < 0) {
    return -1;
  }

  if(ioctl(vi_fd, UI_DEV_CREATE) < 0) {
    return -1;
  }

  return vi_fd;
}

int close_virtual_device() {
  if(ioctl(vi_fd, UI_DEV_DESTROY) < 0) {
    return 1;
  }
  close(vi_fd);
  return 0;
}

}
