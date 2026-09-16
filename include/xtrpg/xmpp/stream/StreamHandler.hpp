#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "xtrpg/xml/node/TagNode.hpp"
#include "xtrpg/xmpp/session/ClientSession.hpp"

namespace xtrpg::xmpp::stream {

class C2SSession; // Forward declaration

/**
 * Representation of a Stateless stream handler.
 */
class StreamHandler {
public:
  virtual ~StreamHandler() = default;

  /**
   * Handler function that is called when a stream receives an inbound Stanza
   * for processing.
   */
  virtual void onStanza(session::ClientSession &clientSession,
                        const xml::node::TagNode &stanza) const = 0;

  /**
   * Lifecycle hook that is called immediately after a new stream is
   * initialized.
   */
  virtual void onStart(session::ClientSession &clientSession) const {}

  /**
   * Lifecycle hook that is called immediately before a stream is
   * terminated.
   */
  virtual void onEnd(session::ClientSession &clientSession) const {}
};

} // namespace xtrpg::xmpp::stream