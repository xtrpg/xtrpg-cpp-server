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
  // If the tokenizer is already in an error state then cease processing.
  if (TokenizationError::NONE != this->_error) {
    return;
  }

  const auto fail = [&](const TokenizationError error) {
    this->_error = error;
  };

  const auto emitToken = [&](const XmlToken &token) {
    std::cout << "[XmlStreamTokenizer] Dispatching XML Token: " << token.content
              << std::endl;
    dispatchObservation(token);
  };

  const auto emitText = [&](const std::string_view text) {
    if (!text.empty()) {
      XmlToken token;
      token.type = TokenType::TEXT_CONTENT;
      token.content = std::string(text);
      emitToken(token);
    }
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
          emitText(this->_buffer);
          this->_buffer.clear();
          this->_state = State::AFTER_OPEN;
        } else {
          this->_buffer += character;
          if (this->_buffer.size() >= __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
            emitText(this->_buffer);
            this->_buffer.clear();
          }
        }
        break;
      case State::AFTER_OPEN:
        this->_currentToken = XmlToken{};
        if (character == '/') {
          this->_buffer.clear();
          this->_currentToken.type = TokenType::CLOSE_TAG;
          this->_state = State::END_TAG_NAME;
        } else if (character == '?') {
          this->_buffer.clear();
          this->_currentToken.type = TokenType::DECLARATION;
          this->_state = State::DECLARATION_NAME;
        } else if (character == '!') {
          this->_specialPrefix.clear();
          this->_state = State::SPECIAL;
        } else if (isNameCharacter(character)) {
          this->_buffer = character;
          this->_currentToken.type = TokenType::OPEN_TAG;
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
          if (this->_buffer.empty()) {
            fail(TokenizationError::MALFORMED_INPUT);
          } else {
            this->_currentToken.content = this->_buffer;
            this->_buffer.clear();
            this->_state = State::START_TAG_BODY;
          }
        } else if (character == '>') {
          if (this->_buffer.empty()) {
            fail(TokenizationError::MALFORMED_INPUT);
          } else {
            this->_currentToken.content = this->_buffer;
            this->_buffer.clear();
            emitToken(this->_currentToken);
            this->_state = State::TEXT;
          }
        } else if (character == '/') {
          if (this->_buffer.empty()) {
            fail(TokenizationError::MALFORMED_INPUT);
          } else {
            this->_currentToken.content = this->_buffer;
            this->_buffer.clear();
            this->_state = State::SELF_CLOSING;
          }
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::START_TAG_BODY:
        if (isWhitespace(character)) {
          break;
        }
        if (character == '>') {
          emitToken(this->_currentToken);
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
          this->_currentToken.attributes[this->_attributeName] = this->_buffer;
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
          this->_currentToken.content = this->_buffer;
          this->_buffer.clear();
          this->_state = State::END_TAG_BODY;
        } else if (character == '>') {
          if (this->_buffer.empty()) {
            fail(TokenizationError::MALFORMED_INPUT);
          } else {
            this->_currentToken.content = this->_buffer;
            this->_buffer.clear();
            emitToken(this->_currentToken);
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
        if (character == '>') {
          emitToken(this->_currentToken);
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
          if (this->_buffer.empty()) {
            fail(TokenizationError::MALFORMED_INPUT);
          } else {
            this->_currentToken.content = this->_buffer;
            this->_buffer.clear();
            this->_state = State::DECLARATION_BODY;
          }
        } else if (character == '?') {
          if (this->_buffer.empty()) {
            fail(TokenizationError::MALFORMED_INPUT);
          } else {
            this->_currentToken.content = this->_buffer;
            this->_buffer.clear();
            this->_state = State::DECLARATION_QUESTION;
          }
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
          this->_currentToken.attributes[this->_attributeName] = this->_buffer;
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
          emitToken(this->_currentToken);
          this->_state = State::TEXT;
        } else {
          fail(TokenizationError::MALFORMED_INPUT);
        }
        break;
      case State::SELF_CLOSING:
        if (character == '>') {
          this->_currentToken.type = TokenType::EMPTY_TAG;
          emitToken(this->_currentToken);
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
          XmlToken token;
          token.type = TokenType::COMMENT;
          token.content = this->_buffer.substr(0, this->_buffer.size() - 3);
          emitToken(token);
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
          emitText(this->_buffer);
          this->_buffer.clear();
          this->_state = State::TEXT;
        } else if (this->_buffer.size() >=
                   __TOKENIZER_MAX_BUFFER_SIZE_IN_CHARS) {
          const auto textSize = this->_buffer.size() - 2;
          emitText(std::string_view(this->_buffer.data(), textSize));
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