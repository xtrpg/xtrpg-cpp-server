#pragma once

#include <string_view>

namespace xtrpg {
template <typename TContext> class IObserver {
public:
  virtual ~IObserver() = default;

  // Generic handler callback
  virtual void onObservation(TContext &ctx) = 0;
};
} // namespace xtrpg