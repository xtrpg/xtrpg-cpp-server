#pragma once

#include "xtrpg/xml/node/IAttributes.hpp"
#include "xtrpg/xml/node/INode.hpp"
#include "xtrpg/xml/node/ITagname.hpp"
#include "xtrpg/xml/node/NodeType.hpp"
#include <stdexcept>
#include <string_view>
#include <utility>

namespace xtrpg::xml::node {

/**
 * Represents an XML declaration node with a tag name and attributes.
 */
class DeclarationNode : public INode, public ITagname, public IAttributes {
public:
  /**
   * Constructs an XML declaration.
   */
  explicit DeclarationNode(std::string tagname)
      : INode(NodeType::DECLARATION), ITagname(std::move(tagname)),
        IAttributes() {
    if (this->getTagname() != "xml") {
      throw std::invalid_argument("XML declaration name must be 'xml'");
    }
  }

  /**
   * Explicitly defaulted copy constructor.
   */
  DeclarationNode(const DeclarationNode &) = default;

  /**
   * Explicitly defaulted move constructor.
   */
  DeclarationNode(DeclarationNode &&) = default;

  /**
   * Explicitly defaulted copy assignment operator.
   */
  DeclarationNode &operator=(const DeclarationNode &) = default;

  /**
   * Explicitly defaulted move assignment operator.
   */
  DeclarationNode &operator=(DeclarationNode &&) = default;

  /**
   * Sets an attribute key/value pair with XML declaration-specific validation.
   * Only allows 'version', 'encoding', and 'standalone' attributes.
   * Version must be '1.0' or '1.1', encoding must be a valid encoding name,
   * and standalone must be 'yes' or 'no'.
   *
   * @throws std::invalid_argument if the attribute is invalid for XML
   * declarations
   */
  void setAttribute(std::string_view key, std::string_view value) override {
    // Validate XML declaration-specific attributes
    if (key == "version") {
      if (value != "1.0" && value != "1.1") {
        throw std::invalid_argument(
            "XML declaration version must be '1.0' or '1.1'");
      }
    } else if (key == "encoding") {
      if (!isValidEncodingName(value)) {
        throw std::invalid_argument("Invalid XML declaration encoding");
      }
    } else if (key == "standalone") {
      if (value != "yes" && value != "no") {
        throw std::invalid_argument(
            "XML declaration standalone must be 'yes' or 'no'");
      }
    } else {
      throw std::invalid_argument(
          "XML declaration does not support attribute: " + std::string(key));
    }

    // Call parent implementation
    IAttributes::setAttribute(key, value);
  }

  /**
   * Serializes the node into an XML formatted string.
   * Note: All attribute validation occurs in setAttribute(), so this method
   * assumes a valid state.
   */
  void serialize(std::ostream &os) const override {
    const auto version = this->getAttribute("version");
    if (!version) {
      throw std::invalid_argument(
          "XML declaration requires a 'version' attribute");
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
      const bool isAllowed =
          isLetter || (character >= '0' && character <= '9') ||
          character == '.' || character == '_' || character == '-';
      if (!isAllowed) {
        return false;
      }
    }
    const unsigned char first = static_cast<unsigned char>(encoding.front());
    return (first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z');
  }
};

} // namespace xtrpg::xml::node