#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include "xtrpg/xml/node/XmlName.hpp"

namespace xtrpg::xml::node {

/**
 * Stores and exposes the canonical name associated with an XML tag.
 */
class ITagname {
public:
  /**
   * Constructor that takes in the canonical name for the tag.
   */
  explicit ITagname(std::string tagname) : _tagname(std::move(tagname)) {
    if (!isValidXmlName(this->_tagname)) {
      throw std::invalid_argument("Invalid XML tag name");
    }
  }

  /**
   * Virtual destructor.
   */
  virtual ~ITagname() = default;

  std::string_view getTagname() const { return this->_tagname; }

private:
  std::string _tagname;
};

} // namespace xtrpg::xml