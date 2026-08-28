#pragma once

#include "xtrpg/xml/node/IAttributes.hpp"
#include "xtrpg/xml/node/INode.hpp"
#include "xtrpg/xml/node/ITagname.hpp"
#include "xtrpg/xml/node/NodeType.hpp"

namespace xtrpg::xml::node {

/**
 * Represents an XML declaration node with a tag name and attributes.
 */
class XmlDeclarationTag : public INode, public ITagname, public IAttributes {
public:
  /**
   * Inline constructor that accepts a tag name.
   */
  explicit XmlDeclarationTag(std::string tagname)
      : INode(NodeType::DECLARATION), ITagname(tagname), IAttributes() {}

  /**
   * Serializes the node into an XML formatted string.
   */
  void serialize(std::ostream &os) const override {
    os << "<?" << this->getTagname();
    IAttributes::serialize(os);
    os << "?>";
  }
};

} // namespace xtrpg::xml::node