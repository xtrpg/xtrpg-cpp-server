#pragma once
#include "xtrpg/core/Jid.hpp"
#include "xtrpg/core/Stanza.hpp"
#include "xtrpg/core/StanzaDirection.hpp"
#include <memory>
#include <string>

using xtrpg::core::Jid;
using xtrpg::core::Stanza;
using xtrpg::core::StanzaDirection;

namespace xtrpg::core {

class IConnectionSession; // Forward declaration

struct StanzaContext {
  // The Core Stanza Payload
  std::shared_ptr<Stanza> stanza{nullptr};

  // Sender Identity Metadata
  Jid getSender() { return this->stanza->from(); }

  // Routing Direction & Flags
  StanzaDirection direction{StanzaDirection::Inbound};

  // Set to true when a consumer handles the stanza
  bool handled{false};

  // Source Session Reference
  // Weak reference to the session that received this stanza.
  // Allows modules/consumers to send direct responses back without circular
  // std::shared_ptr references.
  std::weak_ptr<IConnectionSession> sourceSession;
};

} // namespace xtrpg::core