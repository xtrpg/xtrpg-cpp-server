#pragma once

#include <string_view>

namespace xtrpg {
template <typename TContext> class IObserver {
public:
  virtual ~IObserver() = default;

  virtual std::string_view getName() const = 0;

  // Generic predicate check
  virtual bool canObserve(const TContext &ctx) const = 0;

  // Generic handler callback
  virtual bool observe(TContext &ctx) = 0;
};
} // namespace xtrpg