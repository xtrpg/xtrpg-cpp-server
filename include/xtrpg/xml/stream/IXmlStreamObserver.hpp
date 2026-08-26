#pragma once

#include "xtrpg/xml/IXmlNode.hpp"

namespace xtrpg::xml::stream {

class IXmlStreamObserver {
public:
  /**
   * Virtual destructor.
   */
  virtual ~IXmlStreamObserver() = default;

  virtual void observeNode(const IXmlNode &node) = 0;
};
} // namespace xtrpg::xml