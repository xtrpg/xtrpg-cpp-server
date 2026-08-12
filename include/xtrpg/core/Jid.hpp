#pragma once
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace xtrpg {
namespace core {

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
  Jid(std::string_view node, std::string_view domain,
      std::string_view resource = "");

  /**
   * Static JID factory method.
   */
  static std::optional<Jid> parse(std::string_view jidStr);

  // Getters for JID Components (RFC 7622)
  const std::string &node() const { return this->_node; }
  const std::string &domain() const { return this->_domain; }
  const std::string &resource() const { return this->_resource; }

  // Derived Forms
  std::string bare() const; // "user@domain"
  const std::string &str() const {
    return this->_fullJid;
  } // "user@domain/resource"

  // Helper Predicates
  bool isBare() const { return this->_resource.empty(); }
  bool isFull() const { return !this->_resource.empty(); }
  bool isValid() const { return !this->_domain.empty(); }
  bool isDomainOnly() const {
    return this->_node.empty() && !this->_domain.empty();
  }

  // Comparison Operators (Crucial for std::unordered_map / StanzaRouter
  // lookups)
  bool operator==(const Jid &other) const {
    return this->_fullJid == other._fullJid;
  }
  bool operator!=(const Jid &other) const {
    return this->_fullJid != other._fullJid;
  }
  bool operator<(const Jid &other) const {
    return this->_fullJid < other._fullJid;
  }

  // Stream Output Serialization
  friend std::ostream &operator<<(std::ostream &os, const Jid &jid) {
    return os << jid._fullJid;
  }

private:
  void rebuildCache();
  static std::string normalize(std::string_view str);

  std::string _node;     // e.g., "alice"
  std::string _domain;   // e.g., "example.com" (lowercased)
  std::string _resource; // e.g., "mobile"
  std::string
      _fullJid; // Pre-built full JID string string for zero-copy lookups
};

} // namespace core
} // namespace xtrpg

// Custom std::hash specialization so Jid can be used as key in
// std::unordered_map
namespace std {
template <> struct hash<xtrpg::core::Jid> {
  size_t operator()(const xtrpg::core::Jid &jid) const noexcept {
    return std::hash<std::string>{}(jid.str());
  }
};
} // namespace std