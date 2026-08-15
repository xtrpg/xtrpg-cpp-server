#pragma once

#include <string>

namespace xtrpg::xml {
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