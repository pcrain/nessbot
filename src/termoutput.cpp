#include "termoutput.h"

namespace nessbot {

void init_curses() {
  initscr();                // Initialize the screen
  start_color();            // Use colors
  attron(A_BOLD);           // All text is bold
  use_default_colors();     // Use default colors
  curs_set(0);              // Make cursor invisible
  for (int i = 0; i < 8; ++ i)
    init_pair(i,i,-1);

  getmaxyx(stdscr, _term_h, _term_w);
}

void end_curses() {
  curs_set(1);              // Make cursor visible again
  endwin();
}

void curprint(col color, const char* format, ...) {
  attron(COLOR_PAIR(color));
  va_list argptr;
  va_start(argptr, format);
  std::string str = vformat(format, argptr);
  addstr(str.c_str());
  va_end(argptr);
}

void curprint(const char* format, ...) {
  attron(COLOR_PAIR(7));
  va_list argptr;
  va_start(argptr, format);
  std::string str = vformat(format, argptr);
  addstr(str.c_str());
  va_end(argptr);
}

void curreset() {
  move(0,0);
}

void curclear() {
  refresh();
  clear();
}

}
