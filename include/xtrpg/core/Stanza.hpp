#pragma once

#include "xtrpg/core/Jid.hpp"
#include "xtrpg/core/StanzaCategory.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

using xtrpg::core::Jid;
using xtrpg::core::StanzaCategory;

namespace xtrpg::core {

class Stanza {
public:
  explicit Stanza(StanzaCategory category) : _category(category) {}
  virtual ~Stanza() = default;

  StanzaCategory category() const { return this->_category; }

  // Common Attributes Getters/Setters
  const Jid &to() const { return this->_to; }
  void setTo(Jid to) { this->_to = to; }

  const Jid &from() const { return this->_from; }
  void setFrom(Jid from) { this->_from = from; }

  const std::string &id() const { return this->_id; }
  void setId(std::string_view id) { this->_id = id; }

  // Alternative: Convert directly to std::string via stringstream
  std::string str() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
  }

  // Overload operator<< for output streams (std::cout, std::ostringstream, file
  // streams)
  friend std::ostream &operator<<(std::ostream &os, const Stanza &stanza) {
    stanza.writeXml(os); // Virtual dispatch to concrete child implementation
    return os;
  }

protected:
  // Pure virtual method that derived classes must implement
  virtual void writeXml(std::ostream &os) const = 0;

  // Helper method for derived classes to write standard common attributes
  void writeCommonAttributes(std::ostream &os) const {
    if (!this->_id.empty())
      os << " id='" << this->_id << "'";
    if (this->_to.isValid())
      os << " to='" << this->_to << "'";
    if (this->_from.isValid())
      os << " from='" << this->_from << "'";
  }

private:
  StanzaCategory _category;
  Jid _to;
  Jid _from;
  std::string _id;
};

} // namespace xtrpg::core