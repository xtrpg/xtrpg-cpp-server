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

    session.send("stream:error", [](xml::node::TagNode &streamError) {
      streamError.append("internal-server-error", [](xml::node::TagNode &node) {
        node.set("xmlns", "urn:ietf:params:xml:ns:xmpp-streams");
      });
      streamError.append("text", [](xml::node::TagNode &node) {
        node.set("xmlns", "urn:ietf:params:xml:ns:xmpp-streams");
        node.set("xml:lang", "en");
        node.append("An unexpected error occurred.");
      });
    });

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
