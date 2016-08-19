#ifndef TERMOUTPUT_H_
#define TERMOUTPUT_H_

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <curses.h>

#include <cstdarg>
#include <vector>
#include <string>

#include "util.h"

namespace nessbot {

static int _term_h, _term_w;

enum col {
  BLK,RED,GRN,YLW,BLU,MGN,CYN,WHT
};

void init_curses();
void end_curses();
void curprint(const char* format, ...);
void curprint(col color, const char* format, ...);
void curreset();
void curclear();

}

#endif /* TERMOUTPUT_H_ */
