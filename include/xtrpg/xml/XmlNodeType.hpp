#pragma once

#include <ostream>
#include <vector>

namespace xtrpg::xml {

class XmlNodeType {
public:
  static const XmlNodeType TAG;
  static const XmlNodeType TEXT;

  [[nodiscard]] std::string_view name() const { return this->_name; }

  bool operator==(const XmlNodeType &other) const = default;

  static const std::vector<XmlNodeType> &values() {
    static const std::vector<XmlNodeType> all = {TAG, TEXT};
    return all;
  }

private:
  constexpr explicit XmlNodeType(std::string_view name) : _name(name) {}

  std::string _name;
};

inline const XmlNodeType XmlNodeType::TAG{"Tag"};
inline const XmlNodeType XmlNodeType::TEXT{"Text"};

inline std::ostream &operator<<(std::ostream &os, XmlNodeType type) {
  return os << type.name();
}

} // namespace xtrpg::xml