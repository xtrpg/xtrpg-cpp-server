#pragma once

#include "xtrpg/xml/node/TagNode.hpp"
#include "xtrpg/xmpp/stream/StreamHandler.hpp"

namespace xtrpg::xmpp::stream {

class UnimplementedStreamHandler : public StreamHandler {
public:
  static const UnimplementedStreamHandler &instance() {
    static UnimplementedStreamHandler instance;
    return instance;
  }

  explicit UnimplementedStreamHandler() = default;

  void onStart(session::ClientSession &session) const override {
    session.sendRaw("<stream:stream xmlns='jabber:client' "
                    "xmlns:stream='http://etherx.jabber.org/streams' "
                    "id='err-1' from='example.com' version='1.0'>");

    session.sendRaw(
        "<stream:error><internal-server-error "
        "xmlns='urn:ietf:params:xml:ns:xmpp-streams'/><text "
        "xmlns='urn:ietf:params:xml:ns:xmpp-streams' xml:lang='en'>An "
        "unexpected error occurred.</text></stream:error>");

    session.shutdown();
  }

  void onEnd(session::ClientSession &session) const override {
    session.sendRaw("</stream:stream>");
  }

  void onStanza(session::ClientSession &session,
                const xml::node::TagNode &stanza) const override {
    // Not implemented
  }
};
} // namespace xtrpg::xmpp::stream
