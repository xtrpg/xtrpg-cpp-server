#pragma once

#include <string>

#include "xtrpg/xml/tokenizer/TokenizationError.hpp"

namespace xtrpg::xml::tokenizer {
/**
 * Represents a class that is capable of processing a stream of XML Token
 * events.
 */
class XmlTokenListener {
public:
  virtual ~XmlTokenListener() = default;

  virtual void onXmlToken(const xml::tokenizer::XmlToken &xmlToken) = 0;

  virtual void onTokenizationError(const TokenizationError &error) = 0;
};
} // namespace xtrpg::xml::tokenizer