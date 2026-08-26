#pragma once

namespace xtrpg::xml::tokenizer {

enum class TokenizationError {
  /**
   * Indicates that there is currently no error.
   */
  NONE,

  /**
   * Indicates that the internal buffer has exhausted it's maximum content size.
   * This would be an indication that either a tagname, attribute name or
   * attribute value are too excessive in length to safely processes.
   */
  BUFFER_SIZE_EXHAUSTED,

  /**
   * Indicates that the input contains an invalid XML token sequence.
   */
  MALFORMED_INPUT
};
}