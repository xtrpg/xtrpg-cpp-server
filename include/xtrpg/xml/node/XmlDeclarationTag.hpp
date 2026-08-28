#pragma once

#include "xtrpg/xml/node/IAttributes.hpp"
#include "xtrpg/xml/node/INode.hpp"
#include "xtrpg/xml/node/ITagname.hpp"
#include "xtrpg/xml/node/NodeType.hpp"
#include <stdexcept>
#include <utility>

namespace xtrpg::xml::node {

/**
 * Represents an XML declaration node with a tag name and attributes.
 */
class XmlDeclarationTag : public INode, public ITagname, public IAttributes {
public:
  /**
   * Constructs an XML declaration.
   */
  explicit XmlDeclarationTag(std::string tagname)
      : INode(NodeType::DECLARATION), ITagname(std::move(tagname)),
        IAttributes() {
    if (this->getTagname() != "xml") {
      throw std::invalid_argument("XML declaration name must be 'xml'");
    }
  }

  /**
   * Serializes the node into an XML formatted string.
   */
  void serialize(std::ostream &os) const override {
    const auto version = this->getAttribute("version");
    if (!version || (*version != "1.0" && *version != "1.1")) {
      throw std::invalid_argument(
          "XML declaration requires version 1.0 or 1.1");
    }

    bool hasInvalidAttribute = false;
    this->forEachAttribute([&](std::string_view key, std::string_view) {
      if (key != "version" && key != "encoding" && key != "standalone") {
        hasInvalidAttribute = true;
      }
    });
    if (hasInvalidAttribute) {
      throw std::invalid_argument("Invalid XML declaration attribute");
    }

    if (const auto encoding = this->getAttribute("encoding")) {
      if (!isValidEncodingName(*encoding)) {
        throw std::invalid_argument("Invalid XML declaration encoding");
      }
    }
    if (const auto standalone = this->getAttribute("standalone")) {
      if (*standalone != "yes" && *standalone != "no") {
        throw std::invalid_argument(
            "XML declaration standalone must be 'yes' or 'no'");
      }
    }

    os << "<?xml version=\"" << *version << "\"";
    if (const auto encoding = this->getAttribute("encoding")) {
      os << " encoding=\"" << *encoding << "\"";
    }
    if (const auto standalone = this->getAttribute("standalone")) {
      os << " standalone=\"" << *standalone << "\"";
    }
    os << "?>";
  }

private:
  static bool isValidEncodingName(std::string_view encoding) {
    if (encoding.empty()) {
      return false;
    }
    for (const unsigned char character : encoding) {
      const bool isLetter = (character >= 'A' && character <= 'Z') ||
                            (character >= 'a' && character <= 'z');
      const bool isAllowed = isLetter || (character >= '0' && character <= '9') ||
                             character == '.' || character == '_' ||
                             character == '-';
      if (!isAllowed) {
        return false;
      }
    }
    const unsigned char first = static_cast<unsigned char>(encoding.front());
    return (first >= 'A' && first <= 'Z') ||
           (first >= 'a' && first <= 'z');
  }
};

} // namespace xtrpg::xml::node