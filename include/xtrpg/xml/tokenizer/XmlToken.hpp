#pragma once

#include <string>
#include <unordered_map>

namespace xtrpg::xml::tokenizer {

enum class TokenType {
  /**
   * Represents an OPEN XML Tag with attributes (eg: "<myTag attr='value'>").
   * The content field will equal the name of the tag (eg: "myTag").
   */
  OPEN_TAG,

  /**
   * Represents a CLOSE XML Tag (eg "</myTag>").
   * The content field will equal the name of the tag (eg: "myTag"). The
   * attributes field must be empty.
   */
  CLOSE_TAG,

  /**
   * Represents an empty (or self-closing) XML Tag (eg: "<myTag attr='value'
   * />"). The content field will equal the name of the tag (eg: "myTag").
   */
  EMPTY_TAG,

  /**
   * Represents a declaration tag (eg: "<%xml version='1.0' %>").
   * The content field will equal the name of the tag (eg: "xml").
   */
  DECLARATION,

  /**
   * Represents the raw text conent inside a XML tag. The raw text up to the
   * close tag or the next child element. The content field will equal the
   * contents of the text node. Attributes will generally be empty.
   */
  TEXT_CONTENT,

  /**
   * Represents an XML comment. The content field will equal the comment message
   * itself. The attributes field must be empty.
   */
  COMMENT,
};

struct XmlToken {
  TokenType type;
  std::string content;
  std::unordered_map<std::string, std::string> attributes;
};
} // namespace xtrpg::xml::tokenizer
