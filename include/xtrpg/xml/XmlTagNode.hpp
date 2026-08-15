#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "xtrpg/xml/IAttributes.hpp"
#include "xtrpg/xml/ITagname.hpp"
#include "xtrpg/xml/IXmlNode.hpp"
#include "xtrpg/xml/XmlNodeType.hpp"
#include "xtrpg/xml/XmlTextNode.hpp"

namespace xtrpg::xml {

class XmlTagNode : public IXmlNode, public ITagname, public IAttributes {
public:
  /**
   * Inline constructor that accepts a tag name.
   */
  explicit XmlTagNode(std::string name)
      : IXmlNode(XmlNodeType::TAG), ITagname(name), IAttributes() {}

  /**
   * Returns a reference to the name of the tag.
   */
  const std::string_view name() const { return this->getTagname(); }

  /**
   * Appends a given child node.
   */
  void append(std::shared_ptr<IXmlNode> child) {
    if (child) {
      this->_children.push_back(std::move(child));
    }
  }

  /**
   * Returns a vector of child nodes.
   */
  const std::vector<std::shared_ptr<IXmlNode>> &children() const {
    return this->_children;
  }

  /**
   * Serializes the node into an XML formatted string.
   */
  void serialize(std::ostream &os) const override {
    os << "<" << this->getTagname();

    IAttributes::serialize(os);

    if (this->_children.empty()) {
      os << "/>";
      return;
    }

    os << ">";
    for (const auto &child : this->_children) {
      child->serialize(os);
    }
    os << "</" << this->getTagname() << ">";
  }

private:
  std::vector<std::shared_ptr<IXmlNode>> _children;
};

/**
 * Stream operator overload for easy serialization
 */
inline XmlTagNode &operator<<(XmlTagNode &node,
                              std::shared_ptr<IXmlNode> child) {
  node.append(std::move(child));
  return node;
}

/**
 * Stream operator overload for easy serialization, appends a new Text Node
 * child containing the provided text.
 */
inline XmlTagNode &operator<<(XmlTagNode &node, const std::string &withText) {
  auto textNode = std::make_shared<XmlTextNode>(withText);
  node.append(textNode);
  return node;
}

} // namespace xtrpg::xml