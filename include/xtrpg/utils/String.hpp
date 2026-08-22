#pragma once

#include <algorithm>
#include <string>

namespace xtrpg::utils::string {
// Trim from start (in-place)
void ltrim(std::string &s) { s.erase(0, s.find_first_not_of(" \t\n\r\f\v")); }

// Trim from end (in-place)
void rtrim(std::string &s) { s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1); }

// Trim from both ends (in-place)
void trim(std::string &s) {
  rtrim(s);
  ltrim(s);
}

void toLowerCase(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
}

size_t countUtf8CodePoints(const std::string &str) {
  size_t count = 0;
  for (unsigned char c : str) {
    // If the byte is NOT a UTF-8 continuation byte, it's the start of a
    // character
    if ((c & 0xC0) != 0x80) {
      ++count;
    }
  }
  return count;
}
} // namespace xtrpg::utils::string