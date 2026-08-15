#pragma once

#include "xtrpg/xml/ITagname.hpp"
#include "xtrpg/xml/IXmlNode.hpp"

namespace xtrpg::xml::node {

class XmlEndTag : public IXmlNode, public ITagname {
public:
  /**
   * Inline constructor that accepts a tag name.
   */
  explicit XmlEndTag(std::string tagname)
      : IXmlNode(XmlNodeType::END_TAG), ITagname(tagname) {}

  /**
   * Serializes the node into an XML formatted string.
   */
  void serialize(std::ostream &os) const override {
    os << "</" << this->getTagname() << ">";
  }
};
} // namespace xtrpg::xml::node