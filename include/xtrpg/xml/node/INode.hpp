#pragma once

#include "xtrpg/xml/node/NodeType.hpp"
#include <iostream>

namespace xtrpg::xml::node {

// Forward declaration
class NodeContainer;

/**
 * Defines the common interface and node type metadata for XML nodes.
 */
class INode {
  friend class NodeContainer;

public:
  /**
   * Constructor that takes in a node type.
   */
  explicit INode(NodeType type) : _type(type) {}

  /**
   * Virtual destructor.
   */
  virtual ~INode() = default;

  /**
   * Virtual method to serialize the node into the provided output stream.
   */
  virtual void serialize(std::ostream &os) const = 0;

  /**
   * Returns the type of this node.
   */
  NodeType getNodeType() const { return this->_type; };

  /**
   * Checks if this node is of the specified type.
   */
  bool isType(NodeType type) const { return this->_type == type; }

  /**
   * Returns the parent NodeContainer of this node, or nullptr if this is a
   * root node.
   */
  class NodeContainer *getParent() const { return this->_parent; }

protected:
  /**
   * Sets the parent NodeContainer for this node.
   * Called by NodeContainer when appending children.
   */
  void setParent(class NodeContainer *parent) { this->_parent = parent; }

private:
  NodeType _type;
  class NodeContainer *_parent = nullptr;
};

/**
 * Stream operator overload for easy serialization
 */
inline std::ostream &operator<<(std::ostream &os, const INode &node) {
  node.serialize(os);
  return os;
}

} // namespace xtrpg::xml::node