#pragma once

#include "xtrpg/xml/NodeType.hpp"
#include <iostream>

namespace xtrpg::xml {

class INode {
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
  NodeType type() const { return this->_type; };

private:
  NodeType _type;
};

/**
 * Stream operator overload for easy serialization
 */
inline std::ostream &operator<<(std::ostream &os, const INode &node) {
  node.serialize(os);
  return os;
}

} // namespace xtrpg::xml
