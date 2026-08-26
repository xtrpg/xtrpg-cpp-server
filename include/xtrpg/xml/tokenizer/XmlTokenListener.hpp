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

  /**
   * Indication to the listener instance that it should begin processing a new
   * XML Tag Node, with the given tagname. If the listener already has an open
   * tag then it should assign the existing tag to be the parent of this new
   * tag.
   */
  virtual void openTag(std::string_view tagname) = 0;

  /**
   * Indication that the current tag has finished processing and focus should be
   * returned to it's parent.
   */
  virtual void closeTag() = 0;

  /**
   * Indication to the listener that it should begin processing a new XML
   * Declaration Tag Node, with the given tagname.
   */
  virtual void openDeclaration(std::string_view tagname) = 0;

  /**
   * Indication that the current declaration tag has finished processing and
   * focus should be returned to it's parent.
   */
  virtual void closeDeclaration() = 0;

  /**
   * Defines an attribute (key/value pair) that should be assigned to the
   * current tag or declaration tag.
   */
  virtual void setAttribute(std::string_view name, std::string_view value) = 0;

  /**
   * Defines a block of text that should be applied as a Raw Text Child Node of
   * the current tag.
   */
  virtual void appendText(std::string_view content) = 0;

  /**
   * Defines an error state of the tokenizer.
   */
  virtual void onError(TokenizationError error) = 0;
};
} // namespace xtrpg::xml::tokenizer