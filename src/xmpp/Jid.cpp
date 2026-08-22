#include "xtrpg/xmpp/Jid.hpp"
#include "xtrpg/utils/String.hpp"
#include "xtrpg/xmpp/exception/MalformedJid.hpp"

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>

namespace xtrpg::xmpp {

void Jid::parseFullJid(std::string_view fullJid) {
  // trim the input JID
  std::string trimmedJid{fullJid};
  utils::string::trim(trimmedJid);

  // ensure there is content
  if (trimmedJid.empty()) {
    throw exception::MalformedJid{};
  }

  // extract the parts
  std::string barePart{trimmedJid};
  std::string resourcePart;
  std::string domainPart;
  std::string localPart;

  // Extract resource part (everything after first '/')
  size_t slashPos = trimmedJid.find('/');
  if (slashPos != std::string_view::npos) {
    resourcePart = std::string(trimmedJid.substr(slashPos + 1));
    resourcePart = this->parseResourcePart(resourcePart);
    barePart = trimmedJid.substr(0, slashPos);
  }

  // Extract local part and domain part from bare Part
  size_t atPos = barePart.find('@');
  if (atPos != std::string_view::npos) {
    localPart = this->parseLocalPart(barePart.substr(0, atPos));
    domainPart = this->parseDomainPart(barePart.substr(atPos + 1));
  } else {
    // Domain-only JID (e.g. "conference.example.com")
    domainPart = this->parseDomainPart(barePart);
  }

  // set the different parts
  this->m_domainPart = domainPart;
  this->m_localPart = localPart;
  this->m_resourcePart = resourcePart;

  // Rebuild the cached Full JID
  this->rebuildCachedFullJid();
}

/**
 * Rebuilds the cache for the full JID string.
 */
void Jid::rebuildCachedFullJid() {
  if (this->m_domainPart.empty()) {
    throw exception::MalformedJid{};
  }

  this->m_fullJid.clear();
  this->m_fullJid.reserve(this->m_localPart.size() + this->m_domainPart.size() +
                          this->m_resourcePart.size() + 2);

  if (!this->m_localPart.empty()) {
    this->m_fullJid.append(this->m_localPart).append("@");
  }

  this->m_fullJid.append(this->m_domainPart);

  if (!this->m_resourcePart.empty()) {
    this->m_fullJid.append("/").append(this->m_resourcePart);
  }
}

std::string Jid::parseLocalPart(std::string_view localPart) {
  std::string part{localPart};
  utils::string::toLowerCase(part);
  utils::string::trim(part);

  if (utils::string::countUtf8CodePoints(part) > 1023) {
    throw exception::MalformedJid{};
  }

  return part;
}

std::string Jid::parseDomainPart(std::string_view localPart) {
  std::string part{localPart};
  utils::string::toLowerCase(part);
  utils::string::trim(part);

  if (part.empty() || utils::string::countUtf8CodePoints(part) > 1023) {
    throw exception::MalformedJid{};
  }

  return part;
}

std::string Jid::parseResourcePart(std::string_view localPart) {
  std::string part{localPart};
  utils::string::trim(part);

  if (utils::string::countUtf8CodePoints(part) > 1023) {
    throw exception::MalformedJid{};
  }

  return part;
}

} // namespace xtrpg::xmpp
