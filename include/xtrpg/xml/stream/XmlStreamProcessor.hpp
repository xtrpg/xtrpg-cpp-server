#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "xtrpg/xml/IXmlNode.hpp"
#include "xtrpg/xml/stream/IXmlStreamObserver.hpp"

namespace xtrpg::xml::stream {

class XmlStreamProcessor {
public:
  void registerObserver(std::shared_ptr<IXmlStreamObserver> observer) {
    this->_nodeObservers.push_back(observer);
  }

  void process(std::istream &stream);

private:
  std::vector<std::shared_ptr<IXmlStreamObserver>> _nodeObservers;

  void notifyAll(const IXmlNode &node) {
    for (const auto &observer : this->_nodeObservers) {
      try {
        observer->observeNode(node);
      } catch (const std::exception &e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
      }
    }
  }
};

} // namespace xtrpg::xml::stream