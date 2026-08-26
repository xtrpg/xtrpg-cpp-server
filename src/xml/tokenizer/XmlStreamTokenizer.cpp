#include "xtrpg/xml/tokenizer/XmlStreamTokenizer.hpp"

#include <array>
#include <cctype>

namespace {
bool isNameCharacter(const char character) {
  return std::isalnum(static_cast<unsigned char>(character)) != 0 ||
         character == '_' || character == '-' || character == ':' ||
         character == '.';
}

bool isWhitespace(const char character) {
  return std::isspace(static_cast<unsigned char>(character)) != 0;
}
} // namespace

namespace xtrpg::xml::tokenizer {

void XmlStreamTokenizer::process(std::istream &stream) {
  // If the tokenizer is already in an error state then re-issue the same error.
  if (TokenizationError::NONE != this->_error) {
    if (const auto listener = this->_listener.lock()) {
      listener->onError(this->_error);
    }
    return;
  }

  const auto listener = this->_listener.lock();
  const auto fail = [&](const TokenizationError error) {
    this->_error = error;
    if (listener) {
      listener->onError(error);
    }
  };
  const auto appendText = [&](const std::string_view text) {
    if (!text.empty() && listener) {
      listener->appendText(text);
    }
  };
  const auto openStartTag = [&]() {
    if (this->_buffer.empty()) {
      fail(TokenizationError::MALFORMED_INPUT);
      return;
    }
    if (listener) {
      listener->openTag(this->_buffer);
    }
    this->_buffer.clear();
  };
  const auto openDeclaration = [&]() {
    if (this->_buffer.empty()) {
      fail(TokenizationError::MALFORMED_INPUT);
      return;
    }
    if (listener) {
      listener->openDeclaration(this->_buffer);
    }
    this->_buffer.clear();
  };
  const auto bufferExceeded = [&]() {
    fail(TokenizationError::BUFFER_SIZE_EXHAUSTED);
  };

  this->_buffer.reserve(__TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS);
  this->_attributeName.reserve(128);
  this->_specialPrefix.reserve(7);

  std::array<char, 4096> input;
  while (this->_error == TokenizationError::NONE &&
         (stream.read(input.data(), input.size()) || stream.gcount() > 0)) {
    const auto count = stream.gcount();
    for (std::streamsize index = 0;
         this->_error == TokenizationError::NONE && index < count; ++index) {
      const char character = input[static_cast<std::size_t>(index)];
      switch (this->_state) {
      case State::TEXT:
        if (character == '<') {
          appendText(this->_buffer);
          this->_buffer.clear();
          this->_state = State::AFTER_OPEN;
        } else {
          this->_buffer += character;
          if (this->_buffer.size() >= __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            appendText(this->_buffer);
            this->_buffer.clear();
          }
        }
        break;
      case State::AFTER_OPEN:
        if (character == '/') {
          this->_buffer.clear();
          this->_state = State::END_TAG_NAME;
        } else if (character == '?') {
          this->_buffer.clear();
          this->_state = State::DECLARATION_NAME;
        } else if (character == '!') {
          this->_specialPrefix.clear();
          this->_state = State::SPECIAL;
        } else if (isNameCharacter(character)) {
          this->_buffer = character;
          this->_state = State::START_TAG_NAME;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::START_TAG_NAME:
        if (isNameCharacter(character)) {
          this->_buffer += character;
          if (this->_buffer.size() > __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            bufferExceeded();
          }
        } else if (isWhitespace(character)) {
          openStartTag();
          this->_state = State::START_TAG_BODY;
        } else if (character == '>') {
          openStartTag();
          this->_state = State::TEXT;
        } else if (character == '/') {
          openStartTag();
          this->_state = State::SELF_CLOSING;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::START_TAG_BODY:
        if (isWhitespace(character)) {
          break;
        }
        if (character == '>') {
          this->_state = State::TEXT;
        } else if (character == '/') {
          this->_state = State::SELF_CLOSING;
        } else if (isNameCharacter(character)) {
          this->_buffer = character;
          this->_state = State::ATTRIBUTE_NAME;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::ATTRIBUTE_NAME:
        if (isNameCharacter(character)) {
          this->_buffer += character;
          if (this->_buffer.size() > __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            bufferExceeded();
          }
        } else if (isWhitespace(character)) {
          this->_attributeName = this->_buffer;
          this->_buffer.clear();
          this->_state = State::ATTRIBUTE_AFTER_NAME;
        } else if (character == '=') {
          this->_attributeName = this->_buffer;
          this->_buffer.clear();
          this->_state = State::ATTRIBUTE_VALUE_START;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::ATTRIBUTE_AFTER_NAME:
        if (isWhitespace(character)) {
          break;
        }
        if (character == '=') {
          this->_state = State::ATTRIBUTE_VALUE_START;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::ATTRIBUTE_VALUE_START:
        if (isWhitespace(character)) {
          break;
        }
        if (character == '\'' || character == '"') {
          this->_quote = character;
          this->_buffer.clear();
          this->_state = State::ATTRIBUTE_VALUE;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::ATTRIBUTE_VALUE:
        if (character == this->_quote) {
          if (listener) {
            listener->setAttribute(this->_attributeName, this->_buffer);
          }
          this->_attributeName.clear();
          this->_buffer.clear();
          this->_state = State::START_TAG_BODY;
        } else {
          this->_buffer += character;
          if (this->_buffer.size() > __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            bufferExceeded();
          }
        }
        break;
      case State::END_TAG_NAME:
        if (isNameCharacter(character)) {
          this->_buffer += character;
          if (this->_buffer.size() > __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            bufferExceeded();
          }
        } else if (isWhitespace(character)) {
          this->_state = State::END_TAG_BODY;
        } else if (character == '>') {
          if (this->_buffer.empty()) {
            fail(TokenizationError::MALFORMED_INPUT);
          } else {
            if (listener) {
              listener->closeTag();
            }
            this->_buffer.clear();
            this->_state = State::TEXT;
          }
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::END_TAG_BODY:
        if (isWhitespace(character)) {
          break;
        }
        if (character == '>' && !this->_buffer.empty()) {
          if (listener) {
            listener->closeTag();
          }
          this->_buffer.clear();
          this->_state = State::TEXT;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::DECLARATION_NAME:
        if (isNameCharacter(character)) {
          this->_buffer += character;
          if (this->_buffer.size() > __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            bufferExceeded();
          }
        } else if (isWhitespace(character)) {
          openDeclaration();
          this->_state = State::DECLARATION_BODY;
        } else if (character == '?') {
          openDeclaration();
          this->_state = State::DECLARATION_QUESTION;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::DECLARATION_BODY:
        if (isWhitespace(character)) {
          break;
        }
        if (character == '?') {
          this->_state = State::DECLARATION_QUESTION;
        } else if (isNameCharacter(character)) {
          this->_buffer = character;
          this->_state = State::DECLARATION_ATTRIBUTE_NAME;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::DECLARATION_ATTRIBUTE_NAME:
        if (isNameCharacter(character)) {
          this->_buffer += character;
          if (this->_buffer.size() > __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            bufferExceeded();
          }
        } else if (isWhitespace(character)) {
          this->_attributeName = this->_buffer;
          this->_buffer.clear();
          this->_state = State::DECLARATION_ATTRIBUTE_AFTER_NAME;
        } else if (character == '=') {
          this->_attributeName = this->_buffer;
          this->_buffer.clear();
          this->_state = State::DECLARATION_ATTRIBUTE_VALUE_START;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::DECLARATION_ATTRIBUTE_AFTER_NAME:
        if (isWhitespace(character)) {
          break;
        }
        if (character == '=') {
          this->_state = State::DECLARATION_ATTRIBUTE_VALUE_START;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::DECLARATION_ATTRIBUTE_VALUE_START:
        if (isWhitespace(character)) {
          break;
        }
        if (character == '\'' || character == '"') {
          this->_quote = character;
          this->_buffer.clear();
          this->_state = State::DECLARATION_ATTRIBUTE_VALUE;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::DECLARATION_ATTRIBUTE_VALUE:
        if (character == this->_quote) {
          if (listener) {
            listener->setAttribute(this->_attributeName, this->_buffer);
          }
          this->_attributeName.clear();
          this->_buffer.clear();
          this->_state = State::DECLARATION_BODY;
        } else {
          this->_buffer += character;
          if (this->_buffer.size() > __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            bufferExceeded();
          }
        }
        break;
      case State::DECLARATION_QUESTION:
        if (character == '>') {
          if (listener) {
            listener->closeDeclaration();
          }
          this->_state = State::TEXT;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::SELF_CLOSING:
        if (character == '>') {
          if (listener) {
            listener->closeTag();
          }
          this->_state = State::TEXT;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::SPECIAL:
        this->_specialPrefix += character;
        if (this->_specialPrefix == "--") {
          this->_buffer.clear();
          this->_state = State::COMMENT;
        } else if (this->_specialPrefix == "[CDATA[") {
          this->_buffer.clear();
          this->_state = State::CDATA;
        } else if (std::string_view("--").starts_with(this->_specialPrefix) ||
                   std::string_view("[CDATA[").starts_with(
                       this->_specialPrefix)) {
          break;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::COMMENT:
        this->_buffer += character;
        if (this->_buffer.size() >= 3 && this->_buffer.ends_with("-->")) {
          this->_buffer.clear();
          this->_state = State::TEXT;
        } else if (this->_buffer.size() >
                   __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
          bufferExceeded();
        }
        break;
      case State::CDATA:
        this->_buffer += character;
        if (this->_buffer.size() >= 3 && this->_buffer.ends_with("]]>")) {
          this->_buffer.resize(this->_buffer.size() - 3);
          appendText(this->_buffer);
          this->_buffer.clear();
          this->_state = State::TEXT;
        } else if (this->_buffer.size() >=
                   __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
          const auto textSize = this->_buffer.size() - 2;
          appendText(std::string_view(this->_buffer.data(), textSize));
          const char penultimate = this->_buffer[this->_buffer.size() - 2];
          const char last = this->_buffer[this->_buffer.size() - 1];
          this->_buffer.clear();
          this->_buffer += penultimate;
          this->_buffer += last;
        }
        break;
      }
    }
  }
}

} // namespace xtrpg::xml::tokenizer