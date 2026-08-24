#pragma once

#include <stdexcept>

namespace xtrpg::network::exception {

/**
 * Exception thrown when an operation is attempted on a closed connection.
 */
class ConnectionClosed : public std::runtime_error {
public:
  /**
   * Constructs a ConnectionClosed exception with a default error message.
   */
  explicit ConnectionClosed() : std::runtime_error("Connection is closed") {}
};
} // namespace xtrpg::network::exception