#pragma once
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace xtrpg {
namespace core {

struct JidParts {
  std::string local;    // e.g., "alice"
  std::string domain;   // e.g., "example.com" (lowercased)
  std::string resource; // e.g., "mobile"
};

class Jid {
public:
  /**
   * Default constructor that generates a blank JID.
   */
  Jid() = default;

  /**
   * Constructor that takes in and parsed a JID from a string.
   */
  Jid(std::string_view fullJid);

  /**
   * Constructor that takes in the individual parts to form a JID.
   */
  Jid(std::string_view localpart, std::string_view domainpart,
      std::string_view resourcepart = "");

  /**
   * Static JID facotry method.
   */
  static std::optional<Jid> parse(std::string_view jidStr);

  // Getters for JID Components (RFC 7622)
  const std::string &localpart() const { return this->jidParts.local; }
  const std::string &domain() const { return this->jidParts.domain; }
  const std::string &resource() const { return this->jidParts.resource; }

  // Derived Forms
  std::string bare() const;                           // "user@domain"
  const std::string &full() const { return fullJid; } // "user@domain/resource"

  // Helper Predicates
  bool isBare() const { return jidParts.resource.empty(); }
  bool isFull() const { return !jidParts.resource.empty(); }
  bool isValid() const { return !jidParts.domain.empty(); }
  bool isDomainOnly() const {
    return jidParts.local.empty() && !jidParts.domain.empty();
  }

  // Comparison Operators (Crucial for std::unordered_map / StanzaRouter
  // lookups)
  bool operator==(const Jid &other) const {
    return this->fullJid == other.fullJid;
  }
  bool operator!=(const Jid &other) const {
    return this->fullJid != other.fullJid;
  }
  bool operator<(const Jid &other) const {
    return this->fullJid < other.fullJid;
  }

  // Stream Output Serialization
  friend std::ostream &operator<<(std::ostream &os, const Jid &jid) {
    return os << jid.fullJid;
  }

private:
  void rebuildCache();
  static std::string normalize(std::string_view str);

  JidParts jidParts;
  std::string fullJid; // Pre-built full JID string string for zero-copy lookups
};

} // namespace core
} // namespace xtrpg

// Custom std::hash specialization so Jid can be used as key in
// std::unordered_map
namespace std {
template <> struct hash<xtrpg::core::Jid> {
  size_t operator()(const xtrpg::core::Jid &jid) const noexcept {
    return std::hash<std::string>{}(jid.full());
  }
};
} // namespace std