#pragma once

#include <string_view>

namespace xtrpg::xml::node {

/**
 * Returns whether a string is a valid XML name using the ASCII XML Name
 * character set.
 */
inline bool isValidXmlName(std::string_view name) {
  if (name.empty()) {
    return false;
  }

  const auto isNameStart = [](unsigned char character) {
    return (character >= 'A' && character <= 'Z') ||
           (character >= 'a' && character <= 'z') || character == ':' ||
           character == '_';
  };
  const auto isNameCharacter = [&](unsigned char character) {
    return isNameStart(character) || (character >= '0' && character <= '9') ||
           character == '.' || character == '-';
  };

  if (!isNameStart(static_cast<unsigned char>(name.front()))) {
    return false;
  }

  for (const unsigned char character : name.substr(1)) {
    if (!isNameCharacter(character)) {
      return false;
    }
  }
  return true;
}

} // namespace xtrpg::xml::node
