#pragma once

#include <memory>
#include <vector>

#include "xtrpg/xml/node/INode.hpp"
#include "xtrpg/xml/node/NodeType.hpp"
#include "xtrpg/xml/node/TextNode.hpp"

namespace xtrpg::xml::node {

/**
 * A node design to be a container of other nodes.
 */
class NodeContainer : public INode {
public:
  NodeContainer() : INode(NodeType::CONTAINER) {}

  explicit NodeContainer(NodeType type) : INode(type) {}

  ~NodeContainer() = default;

  /**
   * Appends a given child node.
   *
   * @throws std::invalid_argument if appending would create a cycle.
   */
  void append(std::shared_ptr<INode> child) {
    if (child) {
      // Check if this node is already an ancestor of child by walking up
      // the parent chain. This is O(depth) instead of O(n).
      for (NodeContainer *ancestor = child->getParent(); ancestor != nullptr;
           ancestor = ancestor->getParent()) {
        if (ancestor == this) {
          throw std::invalid_argument("Cannot create a cycle in XML nodes");
        }
      }
      // Set this node as the child's parent and append
      child->setParent(this);
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
   * Checks whether this node has any child nodes.
   *
   * @return true if this node contains at least one child, false otherwise.
   */
  [[nodiscard]] constexpr bool hasChildren() const noexcept {
    return !this->_children.empty();
  }

  /**
   * Serializes all child nodes into the provided output stream.
   */
  void serialize(std::ostream &os) const override {
    for (const auto &child : this->_children) {
      child->serialize(os);
    }
  }

private:
  std::vector<std::shared_ptr<INode>> _children;
};

/**
 * Stream operator overload for easy serialization
 */
inline NodeContainer &operator<<(NodeContainer &node,
                                 std::shared_ptr<INode> child) {
  node.append(std::move(child));
  return node;
}

/**
 * Stream operator overload for easy serialization, appends a new Text Node
 * child containing the provided text.
 */
inline NodeContainer &operator<<(NodeContainer &node,
                                 const std::string &withText) {
  auto textNode = std::make_shared<TextNode>(withText);
  node.append(textNode);
  return node;
}
} // namespace xtrpg::xml::node