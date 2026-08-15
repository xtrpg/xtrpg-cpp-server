#pragma once

#include "xtrpg/xml/IAttributes.hpp"
#include "xtrpg/xml/ITagname.hpp"
#include "xtrpg/xml/IXmlNode.hpp"

namespace xtrpg::xml::node {

class XmlEmptyElementTag : public IXmlNode,
                           public ITagname,
                           public IAttributes {
public:
  /**
   * Inline constructor that accepts a tag name.
   */
  explicit XmlEmptyElementTag(std::string tagname)
      : IXmlNode(XmlNodeType::TAG), ITagname(tagname), IAttributes() {}

  /**
   * Serializes the node into an XML formatted string.
   */
  void serialize(std::ostream &os) const override {
    os << "<" << this->getTagname();
    IAttributes::serialize(os);
    os << "/>";
  }
};

} // namespace xtrpg::xml