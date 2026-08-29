#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "xtrpg/interface/Observable.hpp"
#include "xtrpg/xml/tokenizer/TokenizationError.hpp"
#include "xtrpg/xml/tokenizer/XmlToken.hpp"
#include "xtrpg/xml/tokenizer/XmlTokenListener.hpp"

#ifndef __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS
#define __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS 999999
#endif

namespace xtrpg::xml::tokenizer {
/**
 * A processor class that processing a stream of XML data and fires off
 * tokenization events to a registered listener.
 */
class XmlStreamTokenizer : public interface::Observable<const XmlToken> {
public:
  ~XmlStreamTokenizer() = default;

  /**
   * Consumes the data on the provided stream until it's exhausted.
   */
  void process(std::istream &stream);

private:
  enum class State {
    TEXT,
    AFTER_OPEN,
    START_TAG_NAME,
    START_TAG_BODY,
    ATTRIBUTE_NAME,
    ATTRIBUTE_AFTER_NAME,
    ATTRIBUTE_VALUE_START,
    ATTRIBUTE_VALUE,
    END_TAG_NAME,
    END_TAG_BODY,
    DECLARATION_NAME,
    DECLARATION_BODY,
    DECLARATION_ATTRIBUTE_NAME,
    DECLARATION_ATTRIBUTE_AFTER_NAME,
    DECLARATION_ATTRIBUTE_VALUE_START,
    DECLARATION_ATTRIBUTE_VALUE,
    DECLARATION_QUESTION,
    SELF_CLOSING,
    SPECIAL,
    COMMENT,
    CDATA
  };

  /**
   * Unprocessed stream data that's carried over between process calls.
   */
  std::string _buffer;

  std::string _attributeName;
  std::string _specialPrefix;
  State _state{State::TEXT};
  char _quote{'\0'};

  TokenizationError _error{TokenizationError::NONE};

  /**
   * The current token being parsed.
   */
  XmlToken _currentToken{};
};

} // namespace xtrpg::xml::tokenizer