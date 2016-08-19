#include "neural.h"

namespace nessbot {
  const unsigned NUM_INPUTS = 8;

  NeuralNetwork::NeuralNetwork() {
    num_middle_layers  = 1;
    middle_layer_size  = 50;
    learn_rate         = 0.01l;
    punish_rate        = learn_rate;
    mutation_rate      = 0.01l;
    chaos_rate         = 0.05l;

    last_fitness       = 0;
    last_fitness_delta = 0;
    last_fitness_dd    = 0;
    p1state            = 0;
    firstframe         = true;

    history_length     = 100000;
    history_index      = 0;

    init_network();
  }

  NeuralNetwork::~NeuralNetwork() {
    for (unsigned i = 0; i < num_middle_layers+2; ++i) {
      delete nn_values[i];
      delete nn_errors[i];
      if (i < num_middle_layers+1) {
        delete nn_weights[i];
      }
    }
    delete nn_values;
    delete nn_weights;
    delete num_layer_weights;

    delete history_value;
    delete history_weight;
    delete[] history_count;
  }

  void NeuralNetwork::init_network() {
    history_value = new int[history_length];
    history_weight = new int[history_length];
    for (unsigned i = 0; i < history_length; ++i) {
      history_value[i] = -1;
      history_weight[i] = -1;
    }
    history_count = new int[inname_end][3];
    for (unsigned i = 0; i < inname_end; ++i) {
      history_count[i][0] = 0;
      history_count[i][1] = 0;
      history_count[i][2] = 0;
    }

    nn_values = new precfloat* [num_middle_layers+2];
    nn_errors = new precfloat* [num_middle_layers+2];
    nn_weights = new struct nodeweight* [num_middle_layers+1];
    num_layer_weights = new unsigned [num_middle_layers+1];

    for (unsigned i = 0; i < num_middle_layers+1; ++i) {
      unsigned jmax = ((i == 0) ? NUM_INPUTS : middle_layer_size);
      unsigned kmax = ((i < num_middle_layers) ? middle_layer_size : inname_end);
      nn_values[i] = new precfloat[jmax];
      nn_errors[i] = new precfloat[jmax];
      for (unsigned j = 0; j < jmax; ++j) {
        nn_values[i][j] = 0;
        nn_errors[i][j] = 0;
      }
      nn_weights[i] = new nodeweight[jmax*kmax];
      unsigned counter = 0;
      for (unsigned j = 0; j < jmax; ++j) {
        for (unsigned k = 0; k < kmax; ++k) {
          nn_weights[i][counter] = {j,rand() / (precfloat)RAND_MAX,k};
          ++counter;
        }
      }
      num_layer_weights[i] = counter;
    }
    nn_values[num_middle_layers+1] = new precfloat[inname_end];
    nn_errors[num_middle_layers+1] = new precfloat[inname_end];
    for (unsigned i = 0; i < inname_end; ++i) {
      nn_values[num_middle_layers+1][i] = 0;
      nn_errors[num_middle_layers+1][i] = 0;
    }
  }

  void NeuralNetwork::neural_update(std::vector<unsigned long> inputs, input_name lastoutput) {
    precfloat fitness = compute_fitness(inputs);
    precfloat fd = fitness;

    if (!firstframe) {
      fd -= last_fitness;
      precfloat dd = fd - last_fitness_delta;
      if (fd >= 0 && dd >= 0) { //If we're not improving and our rate of improvement isn't improving
        last_fitness_dd = -1;
        neural_update_layer(num_middle_layers+1,lastoutput,punish_rate,0);
      } else if (fd < 0 && dd < 0) { //If we're improving and our rate of improvement is improving
        last_fitness_dd = 1;
        neural_update_layer(num_middle_layers+1,lastoutput,learn_rate,1);
      } else if (fd < 0) { //If we're at least improving
        last_fitness_dd = 1;
        neural_update_layer(num_middle_layers+1,lastoutput,learn_rate,1);
      } else {
        last_fitness_dd = 0;
      }
    }

    last_fitness = fitness;
    last_fitness_delta = fd;
    firstframe = false;
  }

  precfloat NeuralNetwork::activation(precfloat x) {
    return 1; //TODO: udno this
    precfloat drv = tanh(x);
    return (1.0f - drv*drv);
  }

  void NeuralNetwork::neural_update_layer(unsigned li, int lastoutput, precfloat lr, precfloat target = 0) {
    // nn_weights[li] = weights between layer li and li + 1 (0 = input layer)
    // nn_values[li]  = values of nodes at layer li (0 = input layer)

    compute_layer_error_derivatives(li,lastoutput,target);

    for (unsigned i = 0; i < num_layer_weights[li-1]; ++i) { //For every weight coming into this layer
      unsigned a = nn_weights[li-1][i].a;
      unsigned b = nn_weights[li-1][i].b;
      precfloat weightdelta = nn_values[li-1][a]*lr*nn_errors[li][b];
      nn_weights[li-1][i].w += weightdelta;
    }

    if (li > 1) {
      neural_update_layer(li-1,lastoutput,lr,target); //Update previous layer matching input to this layer
    }
  }

  //li = layer index
  //targetoutput = the value we want to modify
  //targetval = the target value for targetoutput
  void NeuralNetwork::compute_layer_error_derivatives(unsigned li, int targetoutput, precfloat targetval) {
    if (li == num_middle_layers+1) {
      for (int i = 0; i < inname_end; ++i) {
        nn_errors[li][i] = ((i == targetoutput) ? (targetval - nn_values[li][i]) : 0);
        nn_errors[li][i] *= activation(nn_values[li][i]);
      }
      return;
    }

    unsigned cursize = ( (li == 0) ? NUM_INPUTS : middle_layer_size);
    unsigned upsize = ( (li == num_middle_layers) ? inname_end : middle_layer_size);
    unsigned n = 0;

    for (unsigned i = 0; i < cursize; ++i) {
      precfloat errorsum = 0;
      for (unsigned j = 0; j < upsize; ++j) {
        errorsum += nn_weights[li][n].w*nn_errors[li+1][j];
        ++n;
      }
      nn_errors[li][i] = errorsum*activation(nn_values[li][i]);
    }
  }

  // void NeuralNetwork::neural_update_layer(unsigned li, int lastoutput, precfloat lr, precfloat target = 0) {
  //   // nn_weights[li] = weights between layer li and li + 1 (0 = input layer)
  //   // nn_values[li]  = values of nodes at layer li (0 = input layer)

  //   unsigned ulast = lastoutput; //To avoid sign-compare warning
  //   precfloat oval = nn_values[li][ulast]; //Value of output node

  //   for (unsigned i = 0; i < num_layer_weights[li-1]; ++i) { //For every weight coming into this layer
  //     if (nn_weights[li-1][i].b == ulast) { //If the output node is the specified output node
  //       unsigned pi = nn_weights[li-1][i].a; //Index of previous node
  //       precfloat wdelta = lr*nn_values[li-1][pi]*activation(oval);
  //       if (li > num_middle_layers) {
  //         wdelta *= (target-oval);
  //       }
  //       else {
  //         precfloat dsum = 0.0f;
  //         for (unsigned j = 0; j < num_layer_weights[li]; ++j) { //For every weight coming into this layer
  //           if (nn_weights[li][j].a == ulast) {
  //             precfloat nval = nn_values[li+1][nn_weights[li][j].b];
  //             dsum += (nn_weights[li][j].w*(target-nval)*activation(nval));
  //           }
  //         }
  //         wdelta *= dsum;
  //       }
  //       nn_weights[li-1][i].w += wdelta; //Update incoming weight
  //       if (li > 1) {
  //         neural_update_layer(li-1,pi,lr,target); //Update previous layer matching input to this layer
  //       }
  //     }
  //   }
  // }

  precfloat NeuralNetwork::compute_fitness(std::vector<unsigned long> inputs) {
    precfloat xd = hexfloat(inputs[0])/100;
    precfloat yd = hexfloat(inputs[1])/100;

    // precfloat centerstagedistance = std::sqrt(xd*xd+yd*yd);
    precfloat centerstagedistance = xd;

    // curprint(CYN,"D:       %f\n",centerstagedistance);

    precfloat oxd = hexfloat(inputs[3])/100;
    precfloat oyd = hexfloat(inputs[4])/100;

    precfloat opponentdistance = -(std::sqrt((xd-oxd)*(xd-oxd)+(yd-oyd)*(yd-oyd)));

    precfloat fitness = (
        10 * (centerstagedistance*centerstagedistance)
      // + 0  * (opponentdistance*opponentdistance)
    );
    precfloat lfd = fitness - last_fitness;

    curprint(CYN,"Cost:    %f\n",fitness);
    curprint(CYN,"Change:  %f\n",lfd);
    curprint(CYN,"ImpRate: %f\n",lfd - last_fitness_delta);
    return fitness;
  }

  void NeuralNetwork::populate_inputs(std::vector<unsigned long> rawinputs) {
    const int svs[] {358,359,362,363}; //semivulnstates
    const int asize = sizeof(svs) / sizeof(int);

    precfloat xpos  = hexfloat(rawinputs[0]);
    precfloat ypos  = hexfloat(rawinputs[1]);
    p1state     = rawinputs[2];
    precfloat oxpos = hexfloat(rawinputs[3]);
    precfloat oypos = hexfloat(rawinputs[4]);
    precfloat xv    = hexfloat(rawinputs[6]);
    precfloat yv    = hexfloat(rawinputs[7]);

    bool vulnerable = false;
    for (int i = 0; i < asize; ++i) {
      if (p1state == svs[i]) {
        vulnerable = true;
        break;
      }
    }

    nn_values[0][0] = signof(xpos);                                  //"P1 X         ",
    nn_values[0][1] = ((ypos >= 0) ? 1 : -1);                        //"P1 Y         ",
    nn_values[0][2] = signof(oxpos-xpos);                            //"P2-P1 X      ",
    nn_values[0][3] = signof(oypos-ypos);                            //"P2-P1 Y      ",
    nn_values[0][4] = ((xv >= 0) ? 1 : -1);                          //"P1 Xv        ",
    nn_values[0][5] = ((xv == 0) ? 1 : -1);                          //"P1 Static    ",
    nn_values[0][6] = (vulnerable ? 1 : -1);                         //"Helpless     ",
    nn_values[0][7] = ((signof(xv) == signof(oxpos-xpos)) ? 1 : -1); //"Toward P2    ",

    const char* NAMES[NUM_INPUTS] = {
      "P1 X         ",
      "P1 Y         ",
      "P2-P1 X      ",
      "P2-P1 Y      ",
      "P1 Xv        ",
      "P1 Static    ",
      "Helpless     ",
      "Toward P2    "
    };

    for (unsigned i = 0; i < NUM_INPUTS; ++ i) {
      // nn_values[0][i] = 0.5f; //TODO: Nothing updates
      nn_values[0][i] = (nn_values[0][i]/2)+0.5f;
      curprint(MGN,"%s%f\n",NAMES[i],nn_values[0][i]);
    }
    curprint(RED,"P1 State     %u\n",p1state);
  }

  input_name NeuralNetwork::neural_decide(std::vector<unsigned long> rawinputs) {
    populate_inputs(rawinputs);

    unsigned olayer = num_middle_layers+1;

    for (unsigned n = 0; n < olayer; ++n) {
      unsigned nl = (n == num_middle_layers) ? inname_end : middle_layer_size;
      for (unsigned i = 0; i < nl; ++i) {
        nn_values[n+1][i] = 0;
      }
      for (unsigned i = 0; i < num_layer_weights[n]; ++i) {
        precfloat oval = nn_values[n][nn_weights[n][i].a]*nn_weights[n][i].w;
        nn_values[n+1][nn_weights[n][i].b] += oval;
        if (n == olayer-1) {
          // curprint("%f,",nn_weights[n][i].w);refresh();
        }
      }
      for (unsigned i = 0; i < nl; ++i) {
        // nn_values[n+1][i] = tanh(nn_values[n+1][i]);
        if (n == olayer-2) {
          // curprint("%f,",nn_values[n+1][i]);refresh();
        }
      }
    }

    unsigned olen = inname_end;

    precfloat omin, omax;
    for (unsigned i = 0; i < olen; ++i) {
      if (i == 0) {
        omin = omax = nn_values[olayer][i];
      } else if (nn_values[olayer][i] < omin) {
        omin = nn_values[olayer][i];
      } else if (nn_values[olayer][i] > omax) {
        omax = nn_values[olayer][i];
      }
    }

    // Normalize to positive numbers
    precfloat osum = 0;
    for (unsigned i = 0; i < olen; ++ i) {
      nn_values[olayer][i] -= (omin-mutation_rate);
      osum += nn_values[olayer][i];
    }

    // Make sure the weights sum to 1
    for (unsigned i = 0; i < olen; ++ i) {
      nn_values[olayer][i] /= osum;
    }

    // Return if absolutely no preference
    if (omax == omin) {
      return (input_name)(rand() % olen);;
    }

    // Pick the best choice
    if ((rand() / (precfloat)RAND_MAX) > chaos_rate) {
      unsigned best = 0;
      precfloat bval = nn_values[olayer][0];
      for (unsigned i = 1; i < olen; ++i) {
        if (nn_values[olayer][i] > bval) {
          best = i;
          bval = nn_values[olayer][i];
        }
      }
      return (input_name)best;
    }

    // Pick something according to probability distribution
    precfloat ran = (rand() / (precfloat)RAND_MAX);
    for (unsigned i = 0; i < olen; ++ i) {
      ran -= nn_values[olayer][i];
      if (ran <= 0) {
        return (input_name)i;
      }
    }

    return (input_name)0;
  }

  void NeuralNetwork::printoutputs(int output) {
    //Print results of old outputs
    if (history_value[0] >= 0) {
      history_count[history_value[history_index]][history_weight[history_index]] -= 1;
    }
    history_value[history_index] = output;
    history_weight[history_index] = 1-last_fitness_dd;
    history_count[output][1-last_fitness_dd] += 1;
    history_index += 1;
    if (history_index == history_length) {
      history_index = 0;
    }

    if (last_fitness_dd == 1) {
      curprint(GRN,"%s\n",inname_string[output].c_str());
    } else if (last_fitness_dd == 0) {
      curprint(WHT,"%s\n",inname_string[output].c_str());
    } else {
      curprint(RED,"%s\n",inname_string[output].c_str());
    }

    precfloat maxval = nn_values[num_middle_layers+1][0];
    int maxi = 0;
    for (int i = 0; i < inname_end; ++i) {
      precfloat t = nn_values[num_middle_layers+1][i];
      if (t > maxval) {
        maxval = t;
        maxi = i;
      }
    }

    for (int i = 0; i < inname_end; ++i) {
      std::string gc = std::to_string(history_count[i][0]);
      std::string rc = std::to_string(history_count[i][2]);
      std::string nc = std::to_string(history_count[i][1] + history_count[i][0] + history_count[i][2]);

      gc.resize(8,' ');
      nc.resize(8,' ');
      rc.resize(8,' ');

      curprint(WHT,"  %s",inname_string[i].c_str());
      curprint(GRN,"%s",gc.c_str());
      curprint(WHT,"%s",nc.c_str());
      curprint(RED,"%s",rc.c_str());
      curprint( (i == output) ? ((i == maxi) ? GRN : YLW) : BLU,"%f\n",nn_values[num_middle_layers+1][i]);
    }
  }
}
