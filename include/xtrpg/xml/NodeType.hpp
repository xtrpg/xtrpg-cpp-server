#pragma once

#include <ostream>

namespace xtrpg::xml {

class NodeType {
public:
  static const NodeType TAG;
  static const NodeType TEXT;

  [[nodiscard]] std::string_view name() const { return this->_name; }

  bool operator==(const NodeType &other) const = default;

  static const std::vector<NodeType> &values() {
    static const std::vector<NodeType> all = {TAG, TEXT};
    return all;
  }

private:
  constexpr explicit NodeType(std::string_view name) : _name(name) {}

  std::string _name;
};

inline const NodeType NodeType::TAG{"Tag"};
inline const NodeType NodeType::TEXT{"Text"};

inline std::ostream &operator<<(std::ostream &os, NodeType type) {
  return os << type.name();
}

} // namespace xtrpg::xml