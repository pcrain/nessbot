#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <curses.h>

#include <cstdarg>
#include <vector>
#include <string>
#include <string.h>

#include "termoutput.h"

namespace nessbot {

const unsigned char CHUNKSIZE = 4;

int signof(const float x);
// float activation(const float x);
std::string format (const char *fmt, ...);
std::string vformat (const char *fmt, va_list ap);
void fsleep(int frames);
unsigned long hexint(std::string hexstring);
float hexfloat(unsigned long hexint);
void endianfix(char (&array)[4]);
bool file_available(const char* fname);

}

#endif /* UTIL_H_ */
