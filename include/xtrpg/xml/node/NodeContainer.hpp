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

  // Delete copy semantics - ownership transfer is explicit via move
  NodeContainer(const NodeContainer &) = delete;
  NodeContainer &operator=(const NodeContainer &) = delete;

  // Keep move semantics to allow ownership transfer
  NodeContainer(NodeContainer &&) = default;
  NodeContainer &operator=(NodeContainer &&) = default;

  ~NodeContainer() {
    for (INode *child : _children) {
      delete child;
    }
    this->_children.clear();
  };

  /**
   * Appends a given child node.
   *
   * @throws std::invalid_argument if appending would create a cycle.
   */
  void append(INode *ptrChild) {
    if (nullptr == ptrChild) {
      return;
    }

    // Check if this node is already an ancestor of child by walking up
    // the parent chain. This is O(depth) instead of O(n).
    for (NodeContainer *ancestor = ptrChild->getParent(); ancestor != nullptr;
         ancestor = ancestor->getParent()) {
      if (ancestor == this) {
        throw std::invalid_argument("Cannot create a cycle in XML nodes");
      }
    }
    // Set this node as the child's parent and append
    ptrChild->setParent(this);
    this->_children.push_back(ptrChild);
  }

  /**
   * Appends a new TextNode containing the provided string to this container.
   * Exception-safe: if append fails, the TextNode is cleaned up before
   * re-throwing the exception.
   *
   * @param withText the text content for the new TextNode
   * @throws std::invalid_argument if the text contains invalid XML characters
   *         or if appending would create a cycle (though cycles are not
   *         possible with newly created TextNodes).
   */
  void append(const std::string &withText) {
    TextNode *_ptrNode = new TextNode(withText);
    try {
      this->append(_ptrNode);
    } catch (...) {
      delete _ptrNode;
      throw;
    }
  }

  /**
   * Returns a vector of child nodes.
   */
  const std::vector<INode *> &children() const { return this->_children; }

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
  std::vector<INode *> _children;
};

/**
 * Stream operator overload for easy serialization
 */
inline NodeContainer &operator<<(NodeContainer &node, INode *child) {
  node.append(child);
  return node;
}

/**
 * Stream operator overload for easy serialization, appends a new Text Node
 * child containing the provided text.
 */
inline NodeContainer &operator<<(NodeContainer &node,
                                 const std::string &withText) {
  TextNode *_ptrNode = new TextNode(withText);
  try {
    node.append(_ptrNode);
  } catch (...) {
    delete _ptrNode;
    throw;
  }
  return node;
}
} // namespace xtrpg::xml::node