#pragma once

#include <string>

namespace xtrpg::xml::node {

/**
 * Stores and exposes the canonical name associated with an XML tag.
 */
class ITagname {
public:
  /**
   * Constructor that takes in the canonical name for the tag.
   */
  explicit ITagname(std::string tagname) : _tagname(std::move(tagname)) {}

  /**
   * Virtual destructor.
   */
  virtual ~ITagname() = default;

  std::string_view getTagname() const { return this->_tagname; }

private:
  std::string _tagname;
};

} // namespace xtrpg::xml