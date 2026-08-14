#pragma once

#include "xtrpg/xml/INode.hpp"
#include "xtrpg/xml/NodeType.hpp"
#include "xtrpg/xml/TextNode.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace xtrpg::xml {

class TagNode : public INode {
public:
  /**
   * Inline constructor that accepts a tag name.
   */
  explicit TagNode(std::string name)
      : INode(NodeType::TAG), _name(std::move(name)) {}

  /**
   * Returns a reference to the name of the tag.
   */
  const std::string &name() const { return this->_name; }

  /**
   * Sets an attribute key/value pair.
   */
  virtual void attribute(std::string key, std::string value) {
    this->_attributes[std::move(key)] = std::move(value);
  }

  /**
   * Returns the value associated with the given attribute key.
   */
  std::optional<std::string_view> attribute(const std::string &key) const {
    auto it = this->_attributes.find(key);
    if (it != this->_attributes.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  /**
   * Removes a given attribute if it exists, returning the value that was
   * removed.
   */
  std::optional<std::string> remove(const std::string &key) {
    auto node = this->_attributes.extract(key);
    if (node.empty()) {
      return std::nullopt;
    }
    return std::move(node.mapped());
  }

  /**
   * Appends a given child node.
   */
  void append(std::shared_ptr<INode> child) {
    if (child) {
      this->_children.push_back(std::move(child));
    }
  }

  /**
   * Returns a vector of child nodes.
   */
  const std::vector<std::shared_ptr<INode>> &children() const {
    return this->_children;
  }

  /**
   * Serializes the node into an XML formatted string.
   */
  void serialize(std::ostream &os) const override {
    os << "<" << this->_name;
    for (const auto &[attr, val] : this->_attributes) {
      os << " " << attr << "=\"";
      // Escape attribute values
      for (char c : val) {
        if (c == '"')
          os << "&quot;";
        else if (c == '&')
          os << "&amp;";
        else
          os << c;
      }
      os << "\"";
    }

    if (this->_children.empty()) {
      os << " />";
      return;
    }

    os << ">";
    for (const auto &child : this->_children) {
      child->serialize(os);
    }
    os << "</" << this->_name << ">";
  }

private:
  std::string _name;
  std::unordered_map<std::string, std::string> _attributes;
  std::vector<std::shared_ptr<INode>> _children;
};

/**
 * Stream operator overload for easy serialization
 */
inline TagNode &operator<<(TagNode &node, std::shared_ptr<INode> child) {
  node.append(std::move(child));
  return node;
}

/**
 * Stream operator overload for easy serialization, appends a new Text Node
 * child containing the provided text.
 */
inline TagNode &operator<<(TagNode &node, const std::string &withText) {
  auto textNode = std::make_shared<TextNode>(withText);
  node.append(textNode);
  return node;
}

} // namespace xtrpg::xml