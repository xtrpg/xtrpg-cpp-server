#pragma once

#include <ostream>
#include <vector>

namespace xtrpg::xml {

enum class XmlNodeType {
  TAG,
  TEXT,
  DECLARATION,
  START_TAG,
  END_TAG,
  EMPTY_ELEMENT_TAG,
  TEXT_CONTENT
};

inline std::ostream &operator<<(std::ostream &os, XmlNodeType type) {
  return os << type;
}

} // namespace xtrpg::xml