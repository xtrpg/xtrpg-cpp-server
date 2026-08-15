#pragma once

#include "xtrpg/xml/IXmlNode.hpp"
#include "xtrpg/xml/XmlNodeType.hpp"
#include <string>

namespace xtrpg::xml {

class XmlTextContent : public IXmlNode {
public:
  /**
   * Default constructor that generate a blank text node.
   */
  XmlTextContent() : IXmlNode(XmlNodeType::TEXT_CONTENT) {}

  /**
   * Constructor that takes in a copy of the text content to store within this
   * node.
   */
  explicit XmlTextContent(std::string content)
      : IXmlNode(XmlNodeType::TEXT_CONTENT), _content(std::move(content)) {}

  /**
   * Returns a reference to the text content stored on this node.
   */
  const std::string &content() const { return this->_content; }

  /**
   * Sets the text content stored on this node, overriding any previous content.
   */
  void content(std::string withContent) {
    this->_content = std::move(withContent);
  }

  /**
   * Appends the provided string to the end of the stored text content.
   */
  void append(const std::string &withText) { this->_content += withText; }

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
      case '"':
        os << "&quot;";
        break;
      case '\'':
        os << "&apos;";
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
inline XmlTextContent &operator<<(XmlTextContent &node,
                                  const std::string &withText) {
  node.append(withText);
  return node;
}

/**
 * Stream extraction overload: reads line-by-line (standard >> behavior) and
 * appends to the content of the node.
 */
inline std::istream &operator>>(std::istream &is, XmlTextContent &node) {
  std::string temp;
  if (std::getline(is, temp)) {
    node.append(temp);
    node.append("\n");
  }
  return is;
}

} // namespace xtrpg::xml