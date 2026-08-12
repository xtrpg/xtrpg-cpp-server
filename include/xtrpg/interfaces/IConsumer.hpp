#pragma once

#include <string_view>

namespace xtrpg {
template <typename TContext> class IConsumer {
public:
  virtual ~IConsumer() = default;

  virtual std::string_view getName() const = 0;

  // Generic predicate check
  virtual bool canConsume(const TContext &ctx) const = 0;

  // Generic handler callback
  virtual bool consume(TContext &ctx) = 0;
};
} // namespace xtrpg