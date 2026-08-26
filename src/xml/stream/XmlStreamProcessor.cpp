

#include <cctype>
#include <istream>
#include <memory>
#include <string>

#include "xtrpg/xml/nodes/XmlDeclarationTag.hpp"
#include "xtrpg/xml/nodes/XmlEmptyElementTag.hpp"
#include "xtrpg/xml/nodes/XmlEndTag.hpp"
#include "xtrpg/xml/nodes/XmlStartTag.hpp"
#include "xtrpg/xml/nodes/XmlTextContent.hpp"
#include "xtrpg/xml/stream/XmlStreamProcessor.hpp"

namespace xtrpg::xml::stream {

// Helper function to parse attributes into an object deriving from IAttributes
template <typename T> void parseAttributes(std::istream &stream, T &tag) {
  while (stream) {
    // Skip whitespace
    while (stream && std::isspace(stream.peek())) {
      stream.get();
    }

    char next = stream.peek();
    if (next == '>' || next == '?' || next == '/' || next == EOF) {
      break;
    }

    // Read attribute key
    std::string key;
    while (stream && !std::isspace(stream.peek()) && stream.peek() != '=' &&
           stream.peek() != '>' && stream.peek() != '/' &&
           stream.peek() != '?') {
      key += static_cast<char>(stream.get());
    }

    // Skip whitespace leading to '='
    while (stream && std::isspace(stream.peek())) {
      stream.get();
    }

    std::string value;
    if (stream.peek() == '=') {
      stream.get(); // Consume '='
      while (stream && std::isspace(stream.peek())) {
        stream.get();
      }

      if (stream.peek() == '"' || stream.peek() == '\'') {
        char quote = static_cast<char>(stream.get());
        while (stream && stream.peek() != quote) {
          value += static_cast<char>(stream.get());
        }
        if (stream.peek() == quote) {
          stream.get(); // Consume ending quote
        }
      } else {
        // Unquoted attribute value (fallback handling)
        while (stream && !std::isspace(stream.peek()) && stream.peek() != '>') {
          value += static_cast<char>(stream.get());
        }
      }
    }

    if (!key.empty()) {
      tag.setAttribute(key, value);
    }
  }
}

void XmlStreamProcessor::process(std::istream &stream) {
  std::string textBuffer;

  auto flushTextBuffer = [this, &textBuffer]() {
    if (!textBuffer.empty()) {
      XmlTextContent textNode(textBuffer);
      this->notifyAll(textNode);
      textBuffer.clear();
    }
  };

  while (stream.good()) {
    char ch = static_cast<char>(stream.get());
    if (stream.eof()) {
      break;
    }

    if (ch == '<') {
      // Flush accumulated text content prior to entering a tag boundary
      flushTextBuffer();

      char nextChar = static_cast<char>(stream.peek());

      // 1. Declaration Tag: <?xml ... ?>
      if (nextChar == '?') {
        stream.get(); // Consume '?'
        std::string tagname;
        while (stream && !std::isspace(stream.peek()) && stream.peek() != '?') {
          tagname += static_cast<char>(stream.get());
        }

        node::XmlDeclarationTag tag(tagname);
        parseAttributes(stream, tag);

        // Consume closing "?>"
        while (stream && stream.peek() != '>') {
          stream.get();
        }
        if (stream.peek() == '>') {
          stream.get();
        }

        this->notifyAll(tag);
      }
      // 2. End Tag: </tagname>
      else if (nextChar == '/') {
        stream.get(); // Consume '/'
        std::string tagname;
        while (stream && stream.peek() != '>') {
          char c = static_cast<char>(stream.get());
          if (!std::isspace(c)) {
            tagname += c;
          }
        }
        if (stream.peek() == '>') {
          stream.get(); // Consume '>'
        }

        node::XmlEndTag tag(tagname);
        this->notifyAll(tag);
      }
      // 3. Start Tag or Empty Element Tag: <tag ...> or <tag ... />
      else {
        std::string tagname;
        while (stream && !std::isspace(stream.peek()) && stream.peek() != '>' &&
               stream.peek() != '/') {
          tagname += static_cast<char>(stream.get());
        }

        // Parse remaining attributes inside the tag
        bool isEmptyTag = false;

        // Temporary storage for attributes to construct the exact node type
        struct AttrHolder : public IAttributes {};
        AttrHolder attributesHolder;
        parseAttributes(stream, attributesHolder);

        // Check if self-closing ("/>")
        while (stream && std::isspace(stream.peek())) {
          stream.get();
        }

        if (stream.peek() == '/') {
          isEmptyTag = true;
          stream.get(); // Consume '/'
        }
        if (stream.peek() == '>') {
          stream.get(); // Consume '>'
        }

        if (isEmptyTag) {
          node::XmlEmptyElementTag tag(tagname);
          attributesHolder.forEachAttribute(
              [&tag](std::string_view key, std::string_view value) {
                tag.setAttribute(key, value);
              });
          this->notifyAll(tag);
        } else {
          node::XmlStartTag tag(tagname);
          attributesHolder.forEachAttribute(
              [&tag](std::string_view key, std::string_view value) {
                tag.setAttribute(key, value);
              });
          this->notifyAll(tag);
        }
      }
    } else {
      // Accumulate raw text stream
      textBuffer += ch;
    }
  }

  // Flush any leftover text content at the end of stream processing
  flushTextBuffer();
}

} // namespace xtrpg::xml::stream