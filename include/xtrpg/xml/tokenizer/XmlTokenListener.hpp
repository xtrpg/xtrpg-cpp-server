#pragma once

#include <string>

#include "xtrpg/interface/Observer.hpp"
#include "xtrpg/xml/tokenizer/TokenizationError.hpp"

namespace xtrpg::xml::tokenizer {
/**
 * Represents a class that is capable of processing a stream of XML Token
 * events.
 */
class XmlTokenListener
    : public interface::Observer<const xml::tokenizer::XmlToken>,
      public interface::Observer<const TokenizationError> {
public:
  virtual ~XmlTokenListener() = default;

  virtual void onObservation(const xml::tokenizer::XmlToken &xmlToken) = 0;

  virtual void onObservation(const TokenizationError &error) = 0;

  /**
   * Defines an error state of the tokenizer.
   */
  virtual void onError(TokenizationError error) = 0;
};
} // namespace xtrpg::xml::tokenizer