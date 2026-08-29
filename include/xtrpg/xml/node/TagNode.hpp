#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "xtrpg/xml/node/IAttributes.hpp"
#include "xtrpg/xml/node/INode.hpp"
#include "xtrpg/xml/node/ITagname.hpp"
#include "xtrpg/xml/node/NodeContainer.hpp"
#include "xtrpg/xml/node/NodeType.hpp"
#include "xtrpg/xml/node/TextNode.hpp"

namespace xtrpg::xml::node {

/**
 * Represents an XML element with a tag name, attributes, and child nodes.
 */
class TagNode : public INode,
                public ITagname,
                public IAttributes,
                public NodeContainer {
public:
  /**
   * Inline constructor that accepts a tag name.
   */
  explicit TagNode(std::string name)
      : INode(NodeType::TAG), ITagname(name), IAttributes(), NodeContainer() {}

  /**
   * Explicitly defaulted copy constructor.
   */
  TagNode(const TagNode &) = default;

  /**
   * Explicitly defaulted move constructor.
   */
  TagNode(TagNode &&) = default;

  /**
   * Explicitly defaulted copy assignment operator.
   */
  TagNode &operator=(const TagNode &) = default;

  /**
   * Explicitly defaulted move assignment operator.
   */
  TagNode &operator=(TagNode &&) = default;

  /**
   * Returns a reference to the name of the tag.
   */
  const std::string_view name() const { return this->getTagname(); }

  /**
   * Serializes the node into an XML formatted string.
   */
  void serialize(std::ostream &os) const override {
    os << "<" << this->getTagname();

    IAttributes::serialize(os);

    if (!this->hasChildren()) {
      os << "/>";
      return;
    }

    os << ">";
    NodeContainer::serialize(os);
    os << "</" << this->getTagname() << ">";
  }

private:
};
/**
 * Stream operator overload for easy serialization
 */
inline TagNode &operator<<(TagNode &node, std::shared_ptr<INode> child) {
  node.append(std::move(child));
  return node;
}

/**
 * Stream operator overload for easy serialization, appends a new Text Node
 * child containing the provided text.
 */
inline TagNode &operator<<(TagNode &node, const std::string &withText) {
  auto textNode = std::make_shared<TextNode>(withText);
  node.append(textNode);
  return node;
}
} // namespace xtrpg::xml::node