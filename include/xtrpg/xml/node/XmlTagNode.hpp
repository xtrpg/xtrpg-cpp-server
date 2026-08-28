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
#include "xtrpg/xml/node/NodeType.hpp"
#include "xtrpg/xml/node/XmlTextContent.hpp"

namespace xtrpg::xml::node {

/**
 * Represents an XML element with a tag name, attributes, and child nodes.
 */
class XmlTagNode : public INode, public ITagname, public IAttributes {
public:
  /**
   * Inline constructor that accepts a tag name.
   */
  explicit XmlTagNode(std::string name)
      : INode(NodeType::TAG), ITagname(name), IAttributes() {}

  /**
   * Returns a reference to the name of the tag.
   */
  const std::string_view name() const { return this->getTagname(); }

  /**
   * Appends a given child node.
   *
   * @throws std::invalid_argument if the child already contains this node.
   */
  void append(std::shared_ptr<INode> child) {
    if (child) {
      if (containsNode(child.get(), this)) {
        throw std::invalid_argument("Cannot create a cycle in XML nodes");
      }
      this->_children.push_back(std::move(child));
    }
  }

  /**
   * Returns a vector of child nodes.
   */
  const std::vector<std::shared_ptr<INode>> &children() const {
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
  /**
   * Checks whether target is reachable through an XML tag node subtree.
   */
  static bool containsNode(const INode *root, const INode *target) {
    std::vector<const INode *> pending{root};
    std::unordered_set<const INode *> visited;

    while (!pending.empty()) {
      const INode *current = pending.back();
      pending.pop_back();

      if (!current || !visited.insert(current).second) {
        continue;
      }
      if (current == target) {
        return true;
      }

      const auto *tag = dynamic_cast<const XmlTagNode *>(current);
      if (!tag) {
        continue;
      }
      for (const auto &child : tag->_children) {
        pending.push_back(child.get());
      }
    }
    return false;
  }

  std::vector<std::shared_ptr<INode>> _children;
};

/**
 * Stream operator overload for easy serialization
 */
inline XmlTagNode &operator<<(XmlTagNode &node, std::shared_ptr<INode> child) {
  node.append(std::move(child));
  return node;
}

/**
 * Stream operator overload for easy serialization, appends a new Text Node
 * child containing the provided text.
 */
inline XmlTagNode &operator<<(XmlTagNode &node, const std::string &withText) {
  auto textNode = std::make_shared<XmlTextContent>(withText);
  node.append(textNode);
  return node;
}

} // namespace xtrpg::xml::node