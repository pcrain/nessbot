#ifndef NESSBOT_H_
#define NESSBOT_H_

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <exception>

#include "util.h"
#include "termoutput.h"
#include "deviceio.h"
#include "neural.h"
#include "memreader.h"

namespace nessbot {

extern unsigned long p1_state_address;

int run(int argc, char** argv);
void exit_handler(int s);
void init_exit_handler();
int learn_to_melee();
void exit_normal();

}

#endif /* NESSBOT_H_ */
