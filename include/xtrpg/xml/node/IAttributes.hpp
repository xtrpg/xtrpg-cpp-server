#pragma once

#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "xtrpg/xml/node/XmlCharacter.hpp"
#include "xtrpg/xml/node/XmlName.hpp"

namespace xtrpg::xml::node {

/**
 * Provides storage, lookup, mutation, iteration, and serialization for XML
 * attributes.
 */
class IAttributes {
public:
  /**
   * Default constructor.
   */
  IAttributes() {}

  /**
   * Virtual destructor.
   */
  virtual ~IAttributes() = default;

  /**
   * Sets an attribute key/value pair.
   */
  virtual void setAttribute(std::string_view key, std::string_view value) {
    if (!isValidXmlName(key)) {
      throw std::invalid_argument("Invalid XML attribute name");
    }
    if (!isValidXmlCharacterData(value)) {
      throw std::invalid_argument("Invalid character in XML attribute value");
    }
    this->_attributes[std::string(key)] = std::string(value);
  }

  /**
   * Returns a copy of the value associated with the given attribute key.
   */
  std::optional<std::string> getAttribute(const std::string &key) const {
    auto it = this->_attributes.find(key);
    if (it != this->_attributes.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  /**
   * Removes a given attribute if it exists, returning the value that was
   * removed.
   */
  std::optional<std::string> removeAttribute(const std::string &key) {
    auto node = this->_attributes.extract(key);
    if (node.empty()) {
      return std::nullopt;
    }
    return std::move(node.mapped());
  }

  /**
   * Executes a callback function for each key/value attribute pair.
   * The callback can control iteration by returning a boolean value:
   * - Return true to continue iterating to the next attribute
   * - Return false to break early and stop iteration
   *
   * Callback signature: bool(std::string_view key, std::string_view value)
   */
  template <typename Func> void forEachAttribute(Func &&callback) const {
    for (const auto &[key, value] : this->_attributes) {
      if (!callback(std::string_view{key}, std::string_view{value})) {
        break;
      }
    }
  }

  /** Serializes attributes into an XML formatted string in key order. */
  void serialize(std::ostream &os) const {
    for (const auto &[attr, val] : this->_attributes) {
      os << " " << attr << "=\"";
      // Escape attribute values
      for (char c : val) {
        if (c == '"')
          os << "&quot;";
        else if (c == '&')
          os << "&amp;";
        else if (c == '<')
          os << "&lt;";
        else if (c == '\t')
          os << "&#x9;";
        else if (c == '\n')
          os << "&#xA;";
        else if (c == '\r')
          os << "&#xD;";
        else
          os << c;
      }
      os << "\"";
    }
  }

private:
  std::map<std::string, std::string> _attributes;
};

} // namespace xtrpg::xml::node