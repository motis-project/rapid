#pragma once

#include <string>

#include "soro/aliases.h"

namespace soro {

struct train_physics;

seconds time_until_analytical(train_physics const&, km_h target_speed);
meters distance_until_analytical(train_physics const&, km_h target_speed);

std::pair<seconds, meters> train_run_numerical(train_physics const&,
                                               km_h target_speed);
std::pair<seconds, meters> train_run_analytical(train_physics const&,
                                                km_h target_speed);

std::string compute_train_run(train_physics const&, km_h target_speed);

}  // namespace soro
