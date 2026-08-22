#pragma once

#include <stdexcept>

namespace xtrpg::xmpp::exception {
class MalformedJid : public std::invalid_argument {
public:
  explicit MalformedJid() : std::invalid_argument("Malformed JID") {}

  explicit MalformedJid(const std::string &message)
      : std::invalid_argument(message) {}
};
} // namespace xtrpg::xmpp::exception