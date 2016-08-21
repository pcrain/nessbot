#include "neural.h"
#include "memreader.h"

namespace nessbot {
  NeuralNetwork::NeuralNetwork() {
    if (!file_available("./_default_config.json")) {
      num_middle_layers   = 1;
      middle_layer_size   = 50000;
      learn_rate          = 0.0001l;
      punish_rate         = learn_rate;
      mutation_rate       = 0.01l;
      chaos_rate          = 0.05l;

      last_fitness        = 0;
      last_fitness_delta  = 0;
      last_fitness_dd     = 0;
      p1state             = 0;
      firstframe          = true;

      history_length      = 100000;
      history_index       = 0;
      num_inputs          = 7;

      save_default_neural_config("./_default_config.json");
    }
    load_neural_config("./_default_config.json");
    init_network();
    load_network();
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
      unsigned jmax = ((i == 0) ? num_inputs : middle_layer_size);
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

  void NeuralNetwork::neural_update(input_name lastoutput) {
    precfloat fitness = compute_fitness();
    precfloat fd = fitness;

    if (!firstframe) {
      fd -= last_fitness;
      precfloat dd = fd - last_fitness_delta;
      if (fd >= 0 && dd >= 0) { //If we're not improving and showing no signs of improving in the future
        last_fitness_dd = -1;
        neural_update_layer(num_middle_layers+1,lastoutput,punish_rate,0);
      } else if (fd < 0 || dd < 0) { //If we're at least improving
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

  void NeuralNetwork::compute_layer_error_derivatives(unsigned li, int targetoutput, precfloat targetval) {
    if (li == num_middle_layers+1) {
      for (int i = 0; i < inname_end; ++i) {
        nn_errors[li][i] = ((i == targetoutput) ? (targetval - nn_values[li][i]) : 0);
        nn_errors[li][i] *= activation(nn_values[li][i]);
      }
      return;
    }

    unsigned cursize = ( (li == 0) ? num_inputs : middle_layer_size);
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

  precfloat NeuralNetwork::compute_fitness() {
    precfloat lfd, fitness = 0;
    for (unsigned i = 0; i < fitness_weights.size(); ++i) {
      fitness += dolphin_values[fitness_indices[i]].f*fitness_weights[i];
    }
    lfd = fitness - last_fitness;

    curprint(CYN,"Cost:    %f\n",fitness);
    curprint(CYN,"Change:  %f\n",lfd);
    curprint(CYN,"ImpRate: %f\n",lfd - last_fitness_delta);
    return fitness;
  }

  void NeuralNetwork::compute_inputs(std::vector<ram_value> rawinputs) {
    for (unsigned i = 0; i < rawinputs.size(); ++i) {
      unsigned slen = raw_addresses[i].name.size();
      raw_addresses[i].name.insert(slen,15-slen,' ');
      if (raw_addresses[i].vtype == "int") {
        dolphin_values[i].u = rawinputs[i].u;
        curprint(YLW,"%s%u\n",raw_addresses[i].name.c_str(),rawinputs[i].u);
      } else {
        dolphin_values[i].f = rawinputs[i].f;
        curprint(YLW,"%s%f\n",raw_addresses[i].name.c_str(),rawinputs[i].f);
      }
    }

    for (unsigned i = 0; i < input_computations.size(); ++i) {
      unsigned ir = input_computations[i].iret;
      float v1 = dolphin_values[input_computations[i].iarg1].f;
      float v2 = dolphin_values[input_computations[i].iarg2].f;
      switch(input_computations[i].mem_op) {
        case mem_gt:   dolphin_values[ir].f = ((v1 >  v2) ? 1 : -1); break;
        case mem_eq:   dolphin_values[ir].f = ((v1 == v2) ? 1 : -1); break;
        case mem_pos:  dolphin_values[ir].f = ((v1 >  0 ) ? 1 : -1); break;
        case mem_neg:  dolphin_values[ir].f = ((v1 <  0 ) ? 1 : -1); break;
        case mem_zero: dolphin_values[ir].f = ((v1 == 0 ) ? 1 : -1); break;
        case mem_mul:  dolphin_values[ir].f = ( v1 *  v2)          ; break;
        case mem_div:  dolphin_values[ir].f = ( v1 /  v2)          ; break;
        case mem_divc:
          dolphin_values[ir].f =
          ( v1 / std::stof(output_names[input_computations[i].iarg2]));
          break;
      }
    }

    p1state = rawinputs[named_byte_map["P1 State"]].u;
  }

  void NeuralNetwork::populate_neural_inputs() {
    for (unsigned i = 0; i < output_indices.size(); ++i) {
      // nn_values[0][i] = dolphin_values[output_indices[i]].f;
      nn_values[0][i] = (dolphin_values[output_indices[i]].f)/2+0.5f;
      curprint(MGN,"%s%f\n",spaced_names[output_indices[i]].c_str(),nn_values[0][i]);
    }
    curprint(RED,"P1 State     %u\n",p1state);
  }

  input_name NeuralNetwork::neural_decide() {
    populate_neural_inputs();
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
    if ((rand() / (precfloat)RAND_MAX) >= chaos_rate) {
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

  void NeuralNetwork::save_default_neural_config(std::string fname) {
    Json::Value root;

    root["history_length"]    = 100000;
    root["num_middle_layers"] = 1;
    root["middle_layer_size"] = 50000;
    root["learn_rate"]        = 0.0001;
    root["punish_rate"]       = 0.0001;
    root["mutation_rate"]     = 0.01;
    root["chaos_rate"]        = 0.05;
    root["weights_file"]      = "";

    root["raw_addresses"] = Json::Value(Json::arrayValue);
    for (unsigned i = 0; i < raw_addresses.size(); ++i) {
      root["raw_addresses"][i]["friendly_name"] = raw_addresses[i].name;
      root["raw_addresses"][i]["type"] = raw_addresses[i].vtype;
      root["raw_addresses"][i]["address_strings"] = Json::Value(Json::arrayValue);
      for (unsigned j = 0; j < raw_addresses[i].address_strings.size(); ++j) {
        root["raw_addresses"][i]["address_strings"][j] = raw_addresses[i].address_strings[j];
      }
    }

    root["computations"] = Json::Value(Json::arrayValue);

    root["computations"][0]["arg1"]      = "P1 X";
    root["computations"][0]["arg2"]      = "0";
    root["computations"][0]["name"]      = "P1 X+";
    root["computations"][0]["operation"] = ">0";
    root["computations"][0]["output"]    = true;

    root["computations"][1]["arg1"]      = "P1 Y";
    root["computations"][1]["arg2"]      = "0";
    root["computations"][1]["name"]      = "P1 Y+";
    root["computations"][1]["operation"] = ">0";
    root["computations"][1]["output"]    = true;

    root["computations"][2]["arg1"]      = "P2 X";
    root["computations"][2]["arg2"]      = "P1 X";
    root["computations"][2]["name"]      = "P2>P1 X";
    root["computations"][2]["operation"] = ">";
    root["computations"][2]["output"]    = true;

    root["computations"][3]["arg1"]      = "P2 Y";
    root["computations"][3]["arg2"]      = "P1 Y";
    root["computations"][3]["name"]      = "P2>P1 Y";
    root["computations"][3]["operation"] = ">";
    root["computations"][3]["output"]    = true;

    root["computations"][4]["arg1"]      = "P1 VX";
    root["computations"][4]["arg2"]      = "0";
    root["computations"][4]["name"]      = "P1 VX+";
    root["computations"][4]["operation"] = ">0";
    root["computations"][4]["output"]    = true;

    root["computations"][5]["arg1"]      = "P1 VX";
    root["computations"][5]["arg2"]      = "0";
    root["computations"][5]["name"]      = "P1 Static";
    root["computations"][5]["operation"] = "=0";
    root["computations"][5]["output"]    = true;

    root["computations"][6]["arg1"]      = "P1 VX+";
    root["computations"][6]["arg2"]      = "P2>P1 X";
    root["computations"][6]["name"]      = "Toward P2";
    root["computations"][6]["operation"] = "=";
    root["computations"][6]["output"]    = true;

    root["computations"][7]["arg1"]      = "P1 X";
    root["computations"][7]["arg2"]      = "100";
    root["computations"][7]["name"]      = "To Center X";
    root["computations"][7]["operation"] = "/c";
    root["computations"][7]["output"]    = false;

    root["computations"][8]["arg1"]      = "To Center X";
    root["computations"][8]["arg2"]      = "To Center X";
    root["computations"][8]["name"]      = "TCX^2";
    root["computations"][8]["operation"] = "*";
    root["computations"][8]["output"]    = false;

    root["fitness"][0]["name"]           = "TCX^2";
    root["fitness"][0]["weight"]         = 10.0f;

    std::ofstream ofile;
    ofile.open(fname);
    Json::StyledWriter styledWriter;
    ofile << styledWriter.write(root);
    ofile.close();
  }

  //From http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
  void NeuralNetwork::load_neural_config(std::string fname) {
    std::string jsonstring;
    std::ifstream t(fname);
    t.seekg(0, std::ios::end);
    jsonstring.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    jsonstring.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());

    Json::Value root;
    Json::Reader reader;
    reader.parse(jsonstring,root,false);

    history_length     = root["history_length"].asUInt();
    num_middle_layers  = root["num_middle_layers"].asUInt();
    middle_layer_size  = root["middle_layer_size"].asUInt();
    learn_rate         = root["learn_rate"].asDouble();
    punish_rate        = root["punish_rate"].asDouble();
    mutation_rate      = root["mutation_rate"].asDouble();
    chaos_rate         = root["chaos_rate"].asDouble();
    weights_file       = root["weights_file"].asString();

    unsigned rawsize = root["raw_addresses"].size();

    raw_addresses.clear();
    for (unsigned i = 0; i < rawsize; ++i) {
      std::vector<std::string> address_strings;
      for (unsigned j = 0; j < root["raw_addresses"][i]["address_strings"].size(); ++j) {
        address_strings.push_back(root["raw_addresses"][i]["address_strings"][j].asString());
      }
      std::string name = root["raw_addresses"][i]["friendly_name"].asString();
      raw_addresses.push_back({
        name,
        root["raw_addresses"][i]["type"].asString(),
        address_strings
      });
      output_names.push_back(name);
    }

    precompute_offsets();  //Need to recompute RAM offsets for memreader

    ram_value zero_value; zero_value.f = 0;  //Zero value

    input_computations.clear();
    for (unsigned i = 0; i < root["computations"].size(); ++i) {
      unsigned iarg1, iarg2;
      mem_operation mem_op = MEM_OP_MAP[root["computations"][i]["operation"].asString()];

      std::string narg1 = root["computations"][i]["arg1"].asString();
      if (named_byte_map.count(narg1) > 0) {
        iarg1 = named_byte_map[narg1];
      } else {
        named_byte_map[narg1] = rawsize;
        iarg1 = rawsize;
        rawsize += 1;
        output_names.push_back(root["computations"][i]["arg1"].asString());
      }

      std::string narg2 = root["computations"][i]["arg2"].asString();
      if (named_byte_map.count(narg2) > 0) {
        iarg2 = named_byte_map[narg2];
      } else {
        named_byte_map[narg2] = rawsize;
        iarg2 = rawsize;
        rawsize += 1;
        output_names.push_back(root["computations"][i]["arg2"].asString());
      }

      named_byte_map[root["computations"][i]["name"].asString()] = rawsize;

      dolphin_values.clear();
      dolphin_values.resize(rawsize,zero_value);

      if (root["computations"][i]["output"].asBool()) {
        output_indices.push_back(rawsize);
      }
      output_names.push_back(root["computations"][i]["name"].asString());

      input_computations.push_back( {iarg1,iarg2,rawsize,mem_op} );
      rawsize += 1;
      // curprint("%u,%u,%u,%u\n",iarg1,iarg2,rawsize,mem_op); refresh(); fsleep(60);
    }
    num_inputs = output_indices.size();
    // fsleep(60*60*60);

    for (unsigned i = 0; i < root["fitness"].size(); ++i) {
      fitness_indices.push_back(named_byte_map[root["fitness"][i]["name"].asString()]);
      fitness_weights.push_back(root["fitness"][i]["weight"].asFloat());
    }

    for (unsigned i = 0; i < output_names.size(); ++i) {
      std::string sn = output_names[i];
      unsigned len = sn.size();
      sn.insert(len,15-len,' ');
      spaced_names.push_back(sn);
      // curprint(sn.c_str()); refresh(); fsleep(30);
    }
  }

  void NeuralNetwork::save_network() {
    if (weights_file.compare("") == 0) {
      return;
    }
    Json::Value root;

    clear(); curprint("Writing weights to file"); refresh();

    root["weights"] = Json::Value(Json::arrayValue);
    for (unsigned i = 0; i < num_middle_layers+1; ++i) {
      unsigned jmax = ((i == 0) ? num_inputs : middle_layer_size);
      unsigned kmax = ((i < num_middle_layers) ? middle_layer_size : inname_end);
      unsigned counter = 0;
      for (unsigned j = 0; j < jmax; ++j) {
        for (unsigned k = 0; k < kmax; ++k) {
          root["weights"][i][counter] = nn_weights[i][counter].w;
          ++counter;
        }
      }
    }

    std::ofstream ofile;
    ofile.open(weights_file);
    Json::StyledWriter styledWriter;
    ofile << styledWriter.write(root);
    ofile.close();
  }

  void NeuralNetwork::load_network() {
    if (weights_file.compare("") == 0) {
      return;
    }
    if (! file_available(weights_file.c_str())) {
      return;
    }

    clear(); curprint("Loading weights from file"); refresh();

    std::string jsonstring;
    std::ifstream t("_weights.json");
    t.seekg(0, std::ios::end);
    jsonstring.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    jsonstring.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());

    Json::Value root;
    Json::Reader reader;
    reader.parse(jsonstring,root,false);

    for (unsigned i = 0; i < num_middle_layers+1; ++i) {
      unsigned jmax = ((i == 0) ? num_inputs : middle_layer_size);
      unsigned kmax = ((i < num_middle_layers) ? middle_layer_size : inname_end);
      unsigned counter = 0;
      for (unsigned j = 0; j < jmax; ++j) {
        for (unsigned k = 0; k < kmax; ++k) {
          nn_weights[i][counter].w = root["weights"][i][counter].asFloat();
          ++counter;
        }
      }
    }
  }
}
