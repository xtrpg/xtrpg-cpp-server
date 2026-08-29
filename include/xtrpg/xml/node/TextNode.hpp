#pragma once

#include "xtrpg/xml/node/INode.hpp"
#include "xtrpg/xml/node/NodeType.hpp"
#include "xtrpg/xml/node/XmlCharacter.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace xtrpg::xml::node {

/**
 * Represents text content within an XML document and escapes it on output.
 */
class TextNode : public INode {
public:
  /**
   * Default constructor that generate a blank text node.
   */
  TextNode() : INode(NodeType::TEXT_CONTENT) {}

  /**
   * Constructor that takes in a copy of the text content to store within this
   * node.
   */
  explicit TextNode(std::string content)
      : INode(NodeType::TEXT_CONTENT), _content(std::move(content)) {
    if (!isValidXmlCharacterData(this->_content)) {
      throw std::invalid_argument("Invalid character in XML text content");
    }
  }

  /**
   * Returns a reference to the text content stored on this node.
   */
  const std::string &content() const { return this->_content; }

  /**
   * Sets the text content stored on this node, overriding any previous content.
   */
  void setContent(std::string withContent) {
    if (!isValidXmlCharacterData(withContent)) {
      throw std::invalid_argument("Invalid character in XML text content");
    }
    this->_content = std::move(withContent);
  }

  /**
   * Appends the provided string to the end of the stored text content.
   */
  void append(const std::string &withText) {
    if (!isValidXmlCharacterData(withText)) {
      throw std::invalid_argument("Invalid character in XML text content");
    }
    this->_content += withText;
  }

  /**
   * Serializes the text content into the provided output stream, automatically
   * escaping and special XML entities.
   */
  void serialize(std::ostream &os) const override {
    for (char c : this->_content) {
      switch (c) {
      case '<':
        os << "&lt;";
        break;
      case '>':
        os << "&gt;";
        break;
      case '&':
        os << "&amp;";
        break;
      default:
        os << c;
        break;
      }
    }
  }

private:
  std::string _content;
};

/**
 * Stream insertion overload. (node << text).
 */
inline TextNode &operator<<(TextNode &node, const std::string &withText) {
  node.append(withText);
  return node;
}

/**
 * Stream extraction overload: reads one line and appends the characters read
 * to the content of the node without adding a delimiter.
 */
inline std::istream &operator>>(std::istream &is, TextNode &node) {
  std::string temp;
  if (std::getline(is, temp)) {
    node.append(temp);
  }
  return is;
}

} // namespace xtrpg::xml::node