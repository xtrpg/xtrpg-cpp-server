#pragma once

#include <string_view>

namespace xtrpg::interface {
template <typename TContext> class Observer {
public:
  virtual ~Observer() = default;

  // Generic handler callback
  virtual void onObservation(TContext ctx) = 0;
};
} // namespace xtrpg::interface