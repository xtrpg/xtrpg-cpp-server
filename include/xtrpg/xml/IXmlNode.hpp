#pragma once

#include "xtrpg/xml/XmlNodeType.hpp"
#include <iostream>

namespace xtrpg::xml {

class IXmlNode {
public:
  /**
   * Constructor that takes in a node type.
   */
  explicit IXmlNode(XmlNodeType type) : _type(type) {}

  /**
   * Virtual destructor.
   */
  virtual ~IXmlNode() = default;

  /**
   * Virtual method to serialize the node into the provided output stream.
   */
  virtual void serialize(std::ostream &os) const = 0;

  /**
   * Returns the type of this node.
   */
  XmlNodeType type() const { return this->_type; };

private:
  XmlNodeType _type;
};

/**
 * Stream operator overload for easy serialization
 */
inline std::ostream &operator<<(std::ostream &os, const IXmlNode &node) {
  node.serialize(os);
  return os;
}

} // namespace xtrpg::xml
