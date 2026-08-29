#pragma once

#include <string_view>

namespace xtrpg::xml::node {

/**
 * Returns whether a string contains only characters permitted in XML 1.0
 * character data and attribute values.
 */
inline bool isValidXmlCharacterData(std::string_view value) {
  for (const unsigned char character : value) {
    if (character < 0x20 && character != '\t' && character != '\n' &&
        character != '\r') {
      return false;
    }
  }
  return true;
}

} // namespace xtrpg::xml::node
