#pragma once

#include <ostream>

namespace xtrpg::core {

class StanzaCategory {
public:
  /**
   * A request-response (or query-result) mechanism functioning similarly to
   * HTTP GET/POST. It permits a client or server to request data, modify
   * configurations, or fetch metadata like rosters and vCards.
   */
  static const StanzaCategory IQ;

  /**
   * Used to push text or data payloads from one user to another in a
   * "fire-and-forget" fashion without requiring a direct response.
   */
  static const StanzaCategory MESSAGE;

  /**
   * A publish-subscribe mechanism used to broadcast availability status (e.g.,
   * online, away, offline) or handle contact subscription requests.
   */
  static const StanzaCategory PRESENCE;

  [[nodiscard]] std::string_view tagName() const { return this->_tagName; }

  bool operator==(const StanzaCategory &other) const = default;

  static const std::vector<StanzaCategory> &values() {
    static const std::vector<StanzaCategory> all = {IQ, MESSAGE, PRESENCE};
    return all;
  }

private:
  constexpr explicit StanzaCategory(std::string_view tagName)
      : _tagName(tagName) {}

  std::string _tagName;
};

inline const StanzaCategory StanzaCategory::IQ{"iq"};
inline const StanzaCategory StanzaCategory::MESSAGE{"message"};
inline const StanzaCategory StanzaCategory::PRESENCE{"presence"};

inline std::ostream &operator<<(std::ostream &os, StanzaCategory type) {
  return os << type.tagName();
}

} // namespace xtrpg::core
