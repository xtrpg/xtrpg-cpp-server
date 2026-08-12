#pragma once
#include "xtrpg/core/StanzaDirection.hpp"
// #include "xmpp/stanza/Stanza.hpp"
#include <memory>
#include <string>

namespace xtrpg {
namespace core {

class IConnectionSession; // Forward declaration

struct StanzaContext {
  // 1. The Core Stanza Payload
  std::shared_ptr<Stanza> stanza{nullptr};

  // 2. Sender Identity Metadata
  std::string senderBareJid; // e.g. "alice@example.com"
  std::string senderFullJid; // e.g. "alice@example.com/mobile"

  // 3. Routing Direction & Flags
  xtrpg::core::StanzaDirection direction{xtrpg::core::StanzaDirection::Inbound};
  bool handled{false}; // Set to true when a consumer handles the stanza

  // 4. Source Session Reference
  // Weak reference to the session that received this stanza.
  // Allows modules/consumers to send direct responses back without circular
  // std::shared_ptr references.
  std::weak_ptr<IConnectionSession> sourceSession;
};

} // namespace core
} // namespace xtrpg