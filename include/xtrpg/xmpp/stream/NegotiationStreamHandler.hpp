#pragma once

#include "xtrpg/xml/node/TagNode.hpp"
#include "xtrpg/xmpp/stream/StreamHandler.hpp"

namespace xtrpg::xmpp::stream {

class NegotiationStreamHandler : public StreamHandler {
public:
  static const NegotiationStreamHandler &instance() {
    static NegotiationStreamHandler instance;
    return instance;
  }

  explicit NegotiationStreamHandler() = default;

  void onStart(session::ClientSession &session) const override {
    // open the stream
    session.sendRaw(
        "<stream:stream xmlns='jabber:client' "
        "xmlns:stream='http://etherx.jabber.org/streams' id='err-1' "
        "from='example.com' "
        "version='1.0'>");

    // request encrypted
    session.send("stream:features", [](xml::node::TagNode &node) {
      node.append("starttls", [](xml::node::TagNode &node) {
        node.set("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");
        node.append("required", nullptr);
      });
    });
  }

  void onEnd(session::ClientSession &session) const override {
    session.sendRaw("</stream:stream>");
  }

  void onStanza(session::ClientSession &session,
                const xml::node::TagNode &stanza) const override {
    std::string_view name; // = stanza.name();

    if ("starttls" != stanza.name()) {
      // Drop unencrypted/unauthorized stanzas sent prior to TLS
      session.send("stream:error", [](xml::node::TagNode &node) {
        node.append("policy-violation", [](xml::node::TagNode &node) {
          node.set("xmlns", "urn:ietf:params:xml:ns:xmpp-streams");
        });
        node.append("text", [](xml::node::TagNode &node) {
          node.set("xmlns", "urn:ietf:params:xml:ns:xmpp-streams");
          node.set("xml:lang", "en");
          node.append("TLS is required");
        });
      });
      session.shutdown();
      return;
    }

    // Confirm TLS proceed stanza
    session.send("proceed", [](xml::node::TagNode &node) {
      node.set("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");
    });

    // Execute async SSL handshake and transition to unauthenticated phase
    // session.upgrade_to_tls([&session]() {
    // session.reset_parser();
    // Transition will wait for the client's post-TLS <stream:stream> header
    // session.set_state(nullptr); // Waits for next header to instantiate
    // UnauthenticatedState
    // });
  }
};
} // namespace xtrpg::xmpp::stream