#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "xtrpg/xml/node/IAttributes.hpp"
#include "xtrpg/xml/node/ITagname.hpp"
#include "xtrpg/xml/node/NodeContainer.hpp"
#include "xtrpg/xml/node/NodeType.hpp"
#include "xtrpg/xml/node/TextNode.hpp"

namespace xtrpg::xml::node {

/**
 * Represents an XML element with a tag name, attributes, and child nodes.
 */
class TagNode : public ITagname, public IAttributes, public NodeContainer {
public:
  /**
   * Inline constructor that accepts a tag name.
   */
  explicit TagNode(std::string name)
      : ITagname(name), IAttributes(), NodeContainer(NodeType::TAG) {}

  /**
   * Deleted copy constructor - prevents accidental shallow copies of child
   * nodes. Use move semantics for explicit ownership transfer.
   */
  TagNode(const TagNode &) = delete;

  /**
   * Explicitly defaulted move constructor.
   */
  TagNode(TagNode &&) = default;

  /**
   * Deleted copy assignment operator - prevents accidental shallow copies of
   * child nodes. Use move semantics for explicit ownership transfer.
   */
  TagNode &operator=(const TagNode &) = delete;

  /**
   * Explicitly defaulted move assignment operator.
   */
  TagNode &operator=(TagNode &&) = default;

  /**
   * Returns a reference to the name of the tag.
   */
  const std::string_view name() const { return this->getTagname(); }

  /**
   * Appends a new TextNode containing the provided string to this container.
   * Forwards to NodeContainer's string append overload.
   *
   * @param withText the text content for the new TextNode
   * @throws std::invalid_argument if the text contains invalid XML characters
   */
  void append(const std::string &withText) { NodeContainer::append(withText); }

  /**
   * Appends a new TagNode with the provided tag name to this container.
   * The consumer function is called with the new TagNode to allow configuration
   * before it is appended. This pattern enables fluent, nested construction of
   * XML trees.
   *
   * Exception-safe: if the consumer throws or append fails, the TagNode is
   * cleaned up before re-throwing the exception.
   *
   * @param tagname the name for the new TagNode
   * @param consumer a callable that accepts a TagNode& for configuration
   * @throws std::invalid_argument if the tag name is invalid or if the
   *         consumer throws an exception
   *
   * Example usage:
   * @code
   * container.append("error", [](TagNode& error) {
   *   error.append("message", [](TagNode& msg) {
   *     msg.append("Something went wrong");
   *   });
   * });
   * @endcode
   */
  template <typename Consumer>
  void append(const std::string &tagname, Consumer &&consumer) {
    TagNode *tagNode = new TagNode(tagname);
    try {
      consumer(*tagNode);
      this->NodeContainer::append(tagNode);
    } catch (...) {
      delete tagNode;
      throw;
    }
  }

  /**
   * Appends a new TagNode with the provided tag name to this container.
   * This overload accepts nullptr, creating an empty TagNode without calling
   * a consumer function.
   *
   * @param tagname the name for the new TagNode
   * @param consumer nullptr (consumer not provided)
   * @throws std::invalid_argument if the tag name is invalid
   */
  void append(const std::string &tagname, std::nullptr_t) {
    this->NodeContainer::append(new TagNode(tagname));
  }

  /**
   * Serializes the node into an XML formatted string.
   */
  void serialize(std::ostream &os) const override {
    os << "<" << this->getTagname();

    IAttributes::serialize(os);

    if (!this->hasChildren()) {
      os << "/>";
      return;
    }

    os << ">";
    NodeContainer::serialize(os);
    os << "</" << this->getTagname() << ">";
  }

private:
};
/**
 * Stream operator overload for easy serialization
 */
inline TagNode &operator<<(TagNode &node, INode *ptrNode) {
  node.NodeContainer::append(std::move(ptrNode));
  return node;
}

/**
 * Stream operator overload for easy serialization, appends a new Text Node
 * child containing the provided text.
 */
inline TagNode &operator<<(TagNode &node, const std::string &withText) {
  TextNode *_ptrNode = new TextNode(withText);
  try {
    node.NodeContainer::append(_ptrNode);
  } catch (...) {
    delete _ptrNode;
    throw;
  }
  return node;
}
} // namespace xtrpg::xml::node