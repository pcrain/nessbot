#include "util.h"

namespace nessbot {

int signof(const float x) {
  return (x < 0) ? -1 : ( ( x > 0) ? 1 : 0);
}

// float activation(const float x) {
//   float drv = tanh(x);
//   return (1 - drv*drv);
// }

//http://stackoverflow.com/questions/69738/c-how-to-get-fprintf-results-as-a-stdstring-w-o-sprintf#69911
std::string format (const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);
  std::string buf = vformat (fmt, ap);
  va_end (ap);
  return buf;
}

//http://stackoverflow.com/questions/69738/c-how-to-get-fprintf-results-as-a-stdstring-w-o-sprintf#69911
std::string vformat (const char *fmt, va_list ap) {
  // Allocate a buffer on the stack that's big enough for us almost
  // all the time.
  size_t size = 1024;
  char buf[size];

  // Try to vsnprintf into our buffer.
  va_list apcopy;
  va_copy (apcopy, ap);
  unsigned needed = vsnprintf (&buf[0], size, fmt, ap);
  // NB. On Windows, vsnprintf returns -1 if the string didn't fit the
  // buffer.  On Linux & OSX, it returns the length it would have needed.

  if (needed <= size && needed >= 0) {
      // It fit fine the first time, we're done.
      return std::string (&buf[0]);
  } else {
      // vsnprintf reported that it wanted to write more characters
      // than we allotted.  So do a malloc of the right size and try again.
      // This doesn't happen very often if we chose our initial size
      // well.
      std::vector <char> buf;
      size = needed;
      buf.resize (size);
      needed = vsnprintf (&buf[0], size, fmt, apcopy);
      return std::string (&buf[0]);
  }
}

void fsleep(int frames) {
  usleep(16666*frames);
}

unsigned long hexint(std::string hexstring) {
  return std::stoul(hexstring, nullptr, 16);
}

void endianfix(char (&array)[4]) {
  char tmp;
  tmp = array[0];
  array[0] = array[3];
  array[3] = tmp;
  tmp = array[1];
  array[1] = array[2];
  array[2] = tmp;
}

float hexfloat(unsigned long hexint) {
  float f;
  memcpy(&f, &hexint, CHUNKSIZE);
  return f;
}

}
