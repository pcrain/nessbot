#include "nessbot.h"

namespace nessbot {

int run(int argc, char** argv) {
  init_exit_handler();
  init_curses();
  init_device();
  init_memreader();
  srand(time(NULL));

  // curprint("OFFSET = %lu\n",RAMOFFSET); refresh(); fsleep(60);

  // device_test();
  // monitor_game_state();
  learn_to_melee();

  close_memreader();
  close_device();
  end_curses();
  return 0;
}

void learn_to_melee() {
  NeuralNetwork nn;
  std::vector<unsigned long> gs;
  input_name output = (input_name)(-1);

  unsigned long p1state, framenum = 0, lastframe = framenum; //Arbitrary nonzero value

  while (true) {
    curreset(); refresh();
    gs = get_game_state();

    //Update neural network
    if (output >= 0) {
      nn.neural_update(gs,output);
      nn.printoutputs(output);
    }

    //Necide and act upon new action
    output = nn.neural_decide(gs);
    act(output);
    curprint(BLU,"\nDecision: %u\n",output);

    //Wait for next frame
    framenum = get_game_byte(GTADDRESS);
    while( framenum == lastframe ) {
      usleep(10000);
      framenum = get_game_byte(GTADDRESS);
    }

    //Pause if helpless
    p1state = get_game_byte(p1_state_address);
    while (true) {
      // if ( (p1state <= 12) || (p1state > 1000) || (p1state == 341) ) {
      if ( (p1state <= 12) ) {
        usleep(100000);
        p1state = get_game_byte(p1_state_address);
      } else break;
    }

    //Record last frame
    lastframe = framenum;
    curprint(WHT,"%lu",framenum);
  }
}

void exit_handler(int s) {
  close_memreader();
  close_device();
  end_curses();
  exit(0);
}

void init_exit_handler() {
   struct sigaction sigIntHandler;
   sigIntHandler.sa_handler = exit_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;
   sigaction(SIGINT, &sigIntHandler, NULL);
}

}
