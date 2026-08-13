#pragma once

#include <ostream>

namespace xtrpg::xml {

enum class NodeType {
  /**
   * Represents a node formatted as an XML tag.
   */
  TAG,

  /**
   * Represents a node that is pure text.
   */
  TEXT
};

inline std::ostream &operator<<(std::ostream &os, NodeType type) {
  switch (type) {
  case NodeType::TAG:
    return os << "Tag";
  case NodeType::TEXT:
    return os << "Text";
  }
  return os << "Unknown";
}

} // namespace xtrpg::xml