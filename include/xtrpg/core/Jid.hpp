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
  Jid() = default;

  /**
   * Constructor that takes in and parsed a JID from a string.
   */
  Jid(std::string_view fullJid);

  /**
   * Constructor that takes in the individual parts to form a JID.
   *
   * @param[in] node the node/local part of the JID.
   * @param[in] domain the domain part of the JID.
   * @param[in] resource optional resource part of the JID. Default to a blank
   * string if not provided.
   */
  Jid(std::string_view node, std::string_view domain,
      std::string_view resource = "");

  /**
   * Static JID factory method that returns a pointer to a new JID instance.
   */
  static Jid *parse(std::string_view jidStr);

  /**
   * Returns the node/local part of the JID.
   */
  const std::string &node() const { return this->_node; }

  /**
   * Returns the domain part of the JID.
   */
  const std::string &domain() const { return this->_domain; }

  /**
   * Returns the resource part of the JID.
   */
  const std::string &resource() const { return this->_resource; }

  /**
   * Returns the JID in it's bare format (without the resource part).
   */
  std::string bare() const;

  /**
   * Returns the string representation of the full JID.
   */
  const std::string &str() const { return this->_fullJid; }

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

  /**
   * The optional username or specific account name before the `@` symbol.
   *
   * In the example of `alice@example.com/mobile` the node/local part is
   * "alice".
   */
  std::string _node;

  /**
   * The mandatory server address or domain name, located after the `@` and
   * before and `/`.
   *
   * In the example of `alice@example.com/mobile` the domain part is
   * "example.com".
   */
  std::string _domain;

  /**
   * The optional specifier after the `/` symbol used to identify a specific
   * client connection or device, such as "mobile" or "desktop".
   *
   * In the example of `alice@example.com/mobile` the resource part is
   * "mobile".
   */
  std::string _resource;

  /**
   * Pre-built full JID string string for zero-copy lookups.
   */
  std::string _fullJid;
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