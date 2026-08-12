#include "xtrpg/core/Jid.hpp"
#include <algorithm>
#include <cctype>

using xtrpg::core::Jid;

/**
 * Constructor that takes in and parsed a JID from a string.
 */
Jid::Jid(std::string_view fullJid) {
  if (fullJid.empty())
    return;

  // Extract resource part (everything after first '/')
  size_t slashPos = fullJid.find('/');
  std::string_view barePart = fullJid;
  if (slashPos != std::string_view::npos) {
    this->_resource = std::string(fullJid.substr(slashPos + 1));
    barePart = fullJid.substr(0, slashPos);
  }

  // Extract node part and domain part from bare Part
  size_t atPos = barePart.find('@');
  if (atPos != std::string_view::npos) {
    this->_node = normalize(barePart.substr(0, atPos));
    this->_domain = normalize(barePart.substr(atPos + 1));
  } else {
    // Domain-only JID (e.g. "conference.example.com")
    this->_domain = normalize(barePart);
  }

  rebuildCache();
}

/**
 * Constructor that takes in the individual parts to form a JID.
 */
Jid::Jid(std::string_view node, std::string_view domain,
         std::string_view resource)
    : _node(normalize(node)), _domain(normalize(domain)), _resource(resource) {
  rebuildCache();
}

/**
 * Static JID factory method.
 */
Jid *Jid::parse(std::string_view jidStr) {
  Jid *ptrJid = new Jid(jidStr);

  if (ptrJid->isValid()) {
    return ptrJid;
  }

  delete ptrJid;
  return nullptr;
}

/**
 * Returns a derived JID string, excluding the resource part.
 */
std::string Jid::bare() const {
  if (this->_node.empty())
    return this->_domain;
  return this->_node + "@" + this->_domain;
}

/**
 * Rebuilds the cache for the full JID string.
 */
void Jid::rebuildCache() {
  if (this->_domain.empty()) {
    this->_fullJid.clear();
    return;
  }

  this->_fullJid.reserve(this->_node.size() + this->_domain.size() +
                         this->_resource.size() + 2);
  if (!this->_node.empty()) {
    this->_fullJid.append(this->_node).append("@");
  }
  this->_fullJid.append(this->_domain);
  if (!this->_resource.empty()) {
    this->_fullJid.append("/").append(this->_resource);
  }
}

/**
 * Normalizes a given string.
 */
std::string Jid::normalize(std::string_view str) {
  std::string out(str);
  // Lowercase normalization for domain & localpart per RFC 7622 PRECIS
  // stringprep
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return out;
}
