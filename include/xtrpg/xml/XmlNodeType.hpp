#pragma once

#include <ostream>
#include <vector>

namespace xtrpg::xml {

enum class XmlNodeType {
  TAG,
  DECLARATION,
  START_TAG,
  END_TAG,
  EMPTY_ELEMENT_TAG,
  TEXT_CONTENT
};

} // namespace xtrpg::xml