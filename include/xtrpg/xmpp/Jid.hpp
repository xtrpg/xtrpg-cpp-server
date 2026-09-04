#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace xtrpg::xmpp {

class Jid {
public:
  /**
   * Static JID factory method that returns a pointer to a new JID instance.
   *
   * @throws MalformedJid if the provided string does not conform to a valid JID
   * representation.
   */
  [[nodiscard]] static const Jid &parse(std::string_view jidStr) {
    return Jid{jidStr};
  }

  /**
   * Constructs a JID from a full JID string.
   *
   * @throws MalformedJid if the provided string does not conform to a valid JID
   * representation.
   */
  explicit Jid(std::string_view fullJid) { this->parseFullJid(fullJid); }

  /**
   * Constructs a JID from its components.
   *
   * @throws MalformedJid if the provided string does not conform to a valid JID
   * representation.
   */
  explicit Jid(std::string_view localPart, std::string_view domainPart,
               std::string_view resourcePart = {}) {
    this->m_domainPart = this->parseDomainPart(domainPart);
    this->m_localPart = this->parseLocalPart(localPart);
    this->m_resourcePart = this->parseResourcePart(resourcePart);
    this->rebuildCachedFullJid();
  };

  /**
   * Returns a string representing this JID in it's bare format (ie without the
   * resource part).
   */
  [[nodiscard]] const std::string asBareJid() const noexcept {
    return this->toBareJid().str();
  }

  /**
   * Returns a new JID instance containing a Bare format representation of this
   * instance.
   */
  [[nodiscard]] const Jid toBareJid() const noexcept {
    return Jid(this->m_localPart, this->m_domainPart, "");
  }

  /**
   * Returns the local part of the JID.
   */
  [[nodiscard]] const std::string &localPart() const noexcept {
    return this->m_localPart;
  }

  /**
   * Returns the domain part of the JID.
   */
  [[nodiscard]] const std::string &domainPart() const noexcept {
    return this->m_domainPart;
  }

  /**
   * Returns the resource part of the JID.
   */
  [[nodiscard]] const std::string &resourcePart() const noexcept {
    return this->m_resourcePart;
  }

  /**
   * Returns the string representation of the full JID.
   */
  [[nodiscard]] const std::string &str() const { return this->m_fullJid; }

  /**
   * Returns whether the JID contains a local part.
   */
  [[nodiscard]] bool hasLocalPart() const noexcept {
    return !this->m_localPart.empty();
  }

  /**
   * Returns whether the JID contains a resource part.
   */
  [[nodiscard]] bool hasResourcePart() const noexcept {
    return !this->m_resourcePart.empty();
  }

  // Comparison Operators
  bool operator==(const Jid &other) const {
    return this->m_fullJid == other.m_fullJid;
  }
  bool operator!=(const Jid &other) const {
    return this->m_fullJid != other.m_fullJid;
  }
  bool operator<(const Jid &other) const {
    return this->m_fullJid < other.m_fullJid;
  }

  // Stream Output Serialization
  friend std::ostream &operator<<(std::ostream &stream, const Jid &jid) {
    return stream << jid.m_fullJid;
  }

private:
  /**
   * The optional username or specific account name before the `@` symbol.
   *
   * In the example of `alice@example.com/mobile` the local part is "alice".
   */
  std::string m_localPart;

  /**
   * The mandatory server address or domain name, located after the `@` and
   * before and `/`.
   *
   * In the example of `alice@example.com/mobile` the domain part is
   * "example.com".
   */
  std::string m_domainPart;

  /**
   * The optional specifier after the `/` symbol used to identify a specific
   * client connection or device, such as "mobile" or "desktop".
   *
   * In the example of `alice@example.com/mobile` the resource part is
   * "mobile".
   */
  std::string m_resourcePart;

  /**
   * Pre-built full JID string string for zero-copy lookups.
   */
  std::string m_fullJid;

  std::string parseLocalPart(std::string_view localPart);
  std::string parseDomainPart(std::string_view domainPart);
  std::string parseResourcePart(std::string_view resourcePart);
  void parseFullJid(std::string_view fullJid);
  void rebuildCachedFullJid();
};
} // namespace xtrpg::xmpp

// Custom std::hash specialization so Jid can be used as key in
// std::unordered_map
namespace std {
template <> struct hash<xtrpg::xmpp::Jid> {
  size_t operator()(const xtrpg::xmpp::Jid &jid) const noexcept {
    return std::hash<std::string>{}(jid.str());
  }
};
} // namespace std