#include "soro/running_time_calculation.h"

#include <complex>
#include <sstream>

#include "utl/verify.h"

#include "soro/aliases.h"
#include "soro/constants.h"
#include "soro/parse_train_data.h"

namespace soro {

seconds analytical(std::array<train_physics_t, 3> coefficients,
                   seconds const t0, km_h const v0, km_h const v1) {
  auto const& [c0, c1, c2] = coefficients;
  auto const o = std::sqrt(std::complex<float>(4.0F * c0 * c2 - c1 * c1));
  auto const a = std::atan(std::complex<float>{c1 + 2.0F * c2 * v1} / o);
  auto const b = std::atan(std::complex<float>{c1 + 2.0F * c2 * v0} / o);
  return t0 + ((2.0F / (3.6F * o)) * (a - b)).real();
}

seconds time_until_analytical(train_physics const& i, km_h target_speed) {
  target_speed = std::min(target_speed, i.max_speed_);

  auto prev_speed_kmh = 0.0F;
  auto prev_time_s = 0.0F;
  for (auto const& f : i.tractive_force_) {
    if (f.to_ < target_speed) {
      prev_time_s =
          analytical(f.coefficients_, prev_time_s, prev_speed_kmh, f.to_);
      prev_speed_kmh = f.to_;
    } else {
      return analytical(f.coefficients_, prev_time_s, prev_speed_kmh,
                        target_speed);
    }
  }
  assert(false);
  throw std::runtime_error{"error in running time calculation"};
}

meters distance_analytical(std::array<train_physics_t, 3> coefficients,
                           km_h const v0, km_h const v1) {
  std::cout << "Calculating distance from " << v0 << "kmh to " << v1 << "kmh\n";

  auto const& [c0, c1, c2] = coefficients;
  auto const o1 = std::sqrt(std::complex<float>(4.0F * c0 * c2 - c1 * c1));
  auto const o2 = 2.0F * 3.6F * 3.6F * c2;

  auto const f1 = -(2.0F / o1);

  auto const a1 = (f1 * std::atan(std::complex{c1 + 2.0F * c2 * v1} / o1)) / o2;
  auto const b1 = (f1 * std::atan(std::complex{c1 + 2.0F * c2 * v0} / o1)) / o2;

  auto const a2 = (std::log10(c0 + v1 * (c1 + c2 * v1))) / o2;
  auto const b2 = (std::log10(c0 + v0 * (c1 + c2 * v0))) / o2;

  auto const integral1 = a1 - b1;
  auto const integral2 = a2 - b2;

  std::cout << "Integral 1: " << integral1 << '\n';
  std::cout << "Integral 2: " << integral2 << '\n';

  auto const distance = ((a1 - b1) + (a2 - b2));

  std::cout << "Distance Real: " << distance.real()
            << " Distance Imag: " << distance.imag() << '\n';
  return distance.real();
}

meters distance_until_analytical(train_physics const& i, km_h target_speed) {
  target_speed = std::min(target_speed, i.max_speed_);

  auto prev_speed_kmh = 0.0F;
  auto prev_distance = 0.0F;
  for (auto const& f : i.tractive_force_) {
    if (f.to_ < target_speed) {
      prev_distance +=
          distance_analytical(f.coefficients_, prev_speed_kmh, f.to_);
      prev_speed_kmh = f.to_;
    } else {
      prev_distance +=
          distance_analytical(f.coefficients_, prev_speed_kmh, target_speed);
      return prev_distance;
    }
  }
  assert(false);
  throw std::runtime_error{"error in running time calculation"};
}

std::pair<seconds, meters> train_run_analytical(train_physics const& i,
                                                km_h const target_speed) {
  return {time_until_analytical(i, target_speed),
          distance_until_analytical(i, target_speed)};
}

std::pair<seconds, meters> train_run_numerical(train_physics const& i,
                                               km_h const target_speed) {
  auto coefficients_it = begin(i.tractive_force_);
  auto time_sec = 0.0F;
  auto speed_ms_s = 0.0F;
  auto distance_m = 0.0F;

  while (true) {
    auto const speed_km_h = speed_ms_s * M_S_TO_KM_H;

    if (speed_km_h >= target_speed) {
      return {time_sec, distance_m};
    }

    if (speed_km_h >= coefficients_it->to_) {
      ++coefficients_it;
      if (coefficients_it == end(i.tractive_force_)) {
        assert(false);
        throw std::runtime_error{"error in running time calculation"};
      }
    }

    auto const acceleration_ms_s =
        coefficients_it->coefficients_[0] +
        coefficients_it->coefficients_[1] * speed_km_h +
        coefficients_it->coefficients_[2] * speed_km_h * speed_km_h;

    time_sec += DELTA_T;
    speed_ms_s += static_cast<meter_per_second>(acceleration_ms_s * DELTA_T);
    distance_m +=
        (0.5F * acceleration_ms_s * DELTA_T * DELTA_T) + speed_ms_s * DELTA_T;
  }
}

std::string compute_train_run(train_physics const& i, km_h const target_speed) {
  //  std::stringstream out;
  auto& out = std::cout;
  out << "TIME,DISTANCE,SPEED,ANALYTICAL\n";

  auto coefficients_it = begin(i.tractive_force_);
  auto time_sec = 0.0F;
  auto speed_ms_s = 0.0F;
  auto distance_m = 0.0F;

  while (true) {
    auto const speed_km_h = speed_ms_s * M_S_TO_KM_H;

    if (speed_km_h >= target_speed) {
      break;
    }

    if (speed_km_h >= coefficients_it->to_) {
      ++coefficients_it;
      utl::verify(coefficients_it != end(i.tractive_force_),
                  "no force coefficient for {}", speed_ms_s);
    }

    auto const acceleration_ms_s =
        coefficients_it->coefficients_[0] +
        coefficients_it->coefficients_[1] * speed_km_h +
        coefficients_it->coefficients_[2] * speed_km_h * speed_km_h;

    time_sec += DELTA_T;
    distance_m +=
        (0.5F * acceleration_ms_s * DELTA_T * DELTA_T) + speed_ms_s * DELTA_T;
    speed_ms_s += static_cast<meter_per_second>(acceleration_ms_s * DELTA_T);

    //    out << time_sec << "," << distance_m << "," << (speed_ms_s *
    //    M_S_TO_KM_H)
    //        << "," << time_until_analytical(i, speed_km_h) << "\n";

    std::cout << "Time (sec): " << time_sec << '\n';
    std::cout << "Distance (m): " << distance_m << '\n';
    std::cout << "Speed (km/h): " << speed_ms_s * M_S_TO_KM_H << '\n';
    std::cout << "Accel (m/s2): " << acceleration_ms_s << '\n';

    std::cout << "Analytical Time: "
              << time_until_analytical(i, speed_ms_s * M_S_TO_KM_H) << '\n';

    distance_until_analytical(i, speed_ms_s * M_S_TO_KM_H);
  }

  //  return out.str();
  return "";
}

}  // namespace soro