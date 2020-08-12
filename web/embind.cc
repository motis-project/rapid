#include <iostream>
#include <sstream>
#include <string>

#include "emscripten/bind.h"

#include "utl/to_vec.h"
#include "utl/verify.h"

#include "soro/ascii_network_parser.h"
#include "soro/graphviz_output.h"
#include "soro/parse_train_data.h"
#include "soro/propagator.h"
#include "soro/route_train_order.h"
#include "soro/running_time_calculation.h"
#include "soro/timetable_parser.h"

using namespace soro;

struct train_variant {
  std::string name_;
  std::string csv_;
};

std::string simulate(std::string const& network_input,
                     std::string const& trains_input,
                     std::string const& timetable_input) {
  auto const net = parse_network(network_input);
  auto const tt = parse_timetable(net, trains_input, timetable_input);
  compute_route_train_order(tt);
  propagate(tt);

  std::stringstream ss;
  graphiz_output(ss, tt);

  return ss.str();
}

std::vector<train_variant> running_time_calculation(
    std::string const& train_spec) {
  return utl::to_vec(parse_train_data(train_spec), [](auto&& i) {
    return train_variant{.name_ = i.name_,
                         .csv_ = compute_train_run(i, i.max_speed_)};
  });
}

EMSCRIPTEN_BINDINGS(module) {
  emscripten::class_<train_variant>("TrainVariant")
      .property("name", &train_variant::name_)
      .property("csv", &train_variant::csv_);
  emscripten::register_vector<train_variant>("TrainVariantList");
  emscripten::function("soros", &simulate);
  emscripten::function("running_time_calculation", &running_time_calculation);
}