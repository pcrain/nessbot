#ifndef NEURAL_H_
#define NEURAL_H_

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <random>
#include <json/value.h>
#include <json/writer.h>
#include <json/reader.h>
#include <fstream>

#include "deviceio.h"
#include "termoutput.h"

namespace nessbot {

typedef double precfloat;

struct nodeweight {
  unsigned a;
  precfloat w;
  unsigned b;
};

class NeuralNetwork {
private:
  //Settings
  unsigned history_length;
  unsigned num_middle_layers;
  unsigned middle_layer_size;
  precfloat learn_rate;
  precfloat punish_rate;
  precfloat mutation_rate;
  precfloat chaos_rate;

  //Private variables
  precfloat last_fitness;
  precfloat last_fitness_delta;
  int last_fitness_dd;
  int p1state;
  bool firstframe;
  unsigned history_index;

  //Private arrays
  precfloat** nn_values;
  precfloat** nn_errors;
  struct nodeweight** nn_weights;
  unsigned* num_layer_weights;
  int* history_value;
  int* history_weight;
  int (*history_count)[3];
public:
  NeuralNetwork();
  ~NeuralNetwork();
  void init_network();
  void neural_update(std::vector<unsigned long> inputs, input_name lastoutput);
  precfloat compute_fitness(std::vector<unsigned long> inputs);
  void populate_inputs(std::vector<unsigned long> rawinputs);
  void neural_update_layer(unsigned li,int lastoutput,precfloat lr,precfloat target);
  precfloat activation(precfloat f);
  input_name neural_decide();
  void printoutputs(int output);
  void compute_layer_error_derivatives(unsigned li, int targetoutput, precfloat targetval);

  void save_neural_config(std::string fname);
  void load_neural_config(std::string fname);
};

}

#endif /* NEURAL_H_ */
