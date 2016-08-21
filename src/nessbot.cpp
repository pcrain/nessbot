#include "nessbot.h"

namespace nessbot {

int run(int argc, char** argv) {
  int error = 0;

  init_exit_handler();
  init_curses();
  error = init_device();
  if (error < 0) {
    exit_normal();
    if (error == -1)
      std::cout << "No GCN controller in port 1. Is a Wii U adapter plugged in?" << std::endl;
    return error;
  }
  error = init_memreader();
  if (error < 0) {
    exit_normal();
    if (error == -1)
      std::cout << "Could not initialize memory reader. Is Dolphin running?" << std::endl;
    else if (error == -2)
      std::cout << "Could not access Dolphin RAM state. Are you running as root?" << std::endl;
    return error;
  }

  srand(time(NULL));

  // curprint("OFFSET = %lu\n",RAMOFFSET); refresh(); fsleep(60);
  // device_test();
  // monitor_game_state();

  error = learn_to_melee();
  if (error < 0) {
    exit_normal();
    if (error == -1)
      std::cout << "GCN controller unplugged. Terminating." << std::endl;
    else if (error == -3)
      std::cout << "Dolphin closed. Terminating." << std::endl;
    else
      std::cout << "Unknown error: " << error << std::endl;
    return error;
  }

  exit_normal(); return 0;
}

int learn_to_melee() {
  NeuralNetwork nn;
  std::vector<unsigned long> gs;
  input_name output = (input_name)(-1);
  int error = 0;

  unsigned long p1state, framenum = 0, lastframe = framenum; //Arbitrary nonzero value

  while (true) {
    curreset(); refresh();
    gs = get_game_state();
    if (gs.size() == 0) { return -3; } //Dolphin was closed

    //Update neural network
    if (output >= 0) {
      nn.neural_update(gs,output);
      nn.printoutputs(output);
    }

    //Decide and act upon new action
    nn.populate_inputs(gs);
    output = nn.neural_decide();
    error = act(output);
    if (error < 0) { return -1; }
    curprint(BLU,"\nDecision: %u\n",output);

    //Wait for next frame
    error = 0;
    framenum = get_game_byte(GTADDRESS,error);
    if (error < 0) { return -3; }
    while( framenum == lastframe ) {
      usleep(10000);
      framenum = get_game_byte(GTADDRESS,error);
      if (error < 0) { return -3; }
    }

    //Pause if helpless
    p1state = get_game_byte(p1_state_address,error);
    if (error < 0) { return -3; }
    while (true) {
      // if ( (p1state <= 12) || (p1state > 1000) || (p1state == 341) ) {
      // if ( (p1state <= 12) ) {
      if ( (p1state == 0) || (p1state == 12) ) {
        usleep(100000);
        p1state = get_game_byte(p1_state_address,error);
        if (error < 0) { return -3; }
      } else break;
    }

    //Record last frame
    lastframe = framenum;
    curprint(WHT,"%lu",framenum);
  }
  return 0;
}

void exit_normal() {
  close_memreader();
  close_device();
  end_curses();
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
