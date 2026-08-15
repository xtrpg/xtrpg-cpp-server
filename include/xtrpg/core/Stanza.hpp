#pragma once

#include "xtrpg/core/Jid.hpp"
#include "xtrpg/core/StanzaCategory.hpp"
#include "xtrpg/xml/XmlTagNode.hpp"
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace xtrpg::core {

class Stanza : public xml::XmlTagNode {
public:
  /**
   * Instantiates a new StanzaCategory instance.
   */
  explicit Stanza(StanzaCategory category)
      : XmlTagNode(std::string(category.tagName())), _category(category) {}
  virtual ~Stanza() = default;

  StanzaCategory category() const { return this->_category; }

  const bool is(StanzaCategory category) { return this->_category == category; }

  // Common Attributes Getters/Setters
  const Jid &to() const { return this->_to; }
  void to(Jid to) {
    this->_to = to;
    this->setAttribute("to", to.str());
  }

  const Jid &from() const { return this->_from; }
  void from(Jid from) {
    this->_from = from;
    this->setAttribute("from", from.str());
  }

  const std::string &id() const {
    return std::string(xml::XmlTagNode::getAttribute("id").value_or(""));
  }
  void id(std::string_view id) { this->setAttribute("id", std::string(id)); }

  void setAttribute(std::string key, std::string value) {
    if (key == "to") {
      this->_to = Jid(value);
    }

    if (key == "from") {
      this->_from = Jid(value);
    }

    xml::XmlTagNode::setAttribute(key, value);
  }

protected:
private:
  StanzaCategory _category;
  Jid _to;
  Jid _from;
};

} // namespace xtrpg::core