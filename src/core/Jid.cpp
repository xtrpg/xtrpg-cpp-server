#include "xtrpg/core/Jid.hpp"
#include <algorithm>
#include <cctype>

namespace xmpp {

xtrpg::core::Jid::Jid(std::string_view fullJid) {
  if (fullJid.empty())
    return;

  // 1. Extract resourcepart (everything after first '/')
  size_t slashPos = fullJid.find('/');
  std::string_view barePart = fullJid;
  if (slashPos != std::string_view::npos) {
    this->jidParts.resource = std::string(fullJid.substr(slashPos + 1));
    barePart = fullJid.substr(0, slashPos);
  }

  // 2. Extract localpart and domainpart from barePart
  size_t atPos = barePart.find('@');
  if (atPos != std::string_view::npos) {
    this->jidParts.local = normalize(barePart.substr(0, atPos));
    this->jidParts.domain = normalize(barePart.substr(atPos + 1));
  } else {
    // Domain-only JID (e.g. "conference.example.com")
    this->jidParts.domain = normalize(barePart);
  }

  rebuildCache();
}

xtrpg::core::Jid::Jid(std::string_view localpart, std::string_view domainpart,
                      std::string_view resourcepart) {
  this->jidParts.local = normalize(localpart);
  this->jidParts.domain = normalize(domainpart);
  this->jidParts.resource = resourcepart;
  rebuildCache();
}

std::optional<xtrpg::core::Jid>
xtrpg::core::Jid::parse(std::string_view jidStr) {
  Jid jid(jidStr);
  if (jid.isValid()) {
    return jid;
  }
  return std::nullopt;
}

std::string xtrpg::core::Jid::bare() const {
  if (this->jidParts.local.empty())
    return this->jidParts.domain;
  return this->jidParts.local + "@" + this->jidParts.domain;
}

void xtrpg::core::Jid::rebuildCache() {
  if (this->jidParts.domain.empty()) {
    this->fullJid.clear();
    return;
  }

  this->fullJid.reserve(this->jidParts.local.size() +
                        this->jidParts.domain.size() +
                        this->jidParts.resource.size() + 2);
  if (!this->jidParts.local.empty()) {
    this->fullJid.append(this->jidParts.local).append("@");
  }
  this->fullJid.append(this->jidParts.domain);
  if (!this->jidParts.resource.empty()) {
    this->fullJid.append("/").append(this->jidParts.resource);
  }
}

std::string xtrpg::core::Jid::normalize(std::string_view str) {
  std::string out(str);
  // Lowercase normalization for domain & localpart per RFC 7622 PRECIS
  // stringprep
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return out;
}

} // namespace xmpp