#include "soro/route_train_order.h"

#include "utl/pairwise.h"

namespace soro {

train_order_map compute_route_train_order(timetable const& tt) {
  train_order_map route_train_order;

  // Build map: (route from, route to) -> [train, ...]
  for (auto& [name, t] : tt) {
    for (auto const& r : t->routes_) {
      if (r->from_ != nullptr) {
        auto& train_order = route_train_order[{r->from_, r->to_}];
        train_order.emplace(
            std::lower_bound(begin(train_order), end(train_order), r.get(),
                             [](route const* a, route const* b) {
                               return a->from_time_ < b->from_time_;
                             }),
            r.get());
      }
    }
  }

  // Build dependencies.
  for (auto const& [fromto, routes] : route_train_order) {
    for (auto const [r1, r2] : utl::pairwise(routes)) {
      if (r2->pred_ != nullptr && r2->pred_->from_ != nullptr) {
        r1->out_.emplace(r2->pred_);
        r2->pred_->in_.emplace(r1);
      }
    }
  }

  return route_train_order;
}

}  // namespace soro