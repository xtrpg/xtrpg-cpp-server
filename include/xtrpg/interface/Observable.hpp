#pragma once

#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include "xtrpg/interface/Observer.hpp"

namespace xtrpg::interface {
template <typename TContext> class Observable {
public:
  virtual ~Observable() {
    if (nullptr != this->_ptrObserver) {
      std::cerr << "Observable not removed from an instance. This may lead to "
                   "memory leaks."
                << std::endl;
    }
  };

  void setObserver(Observer<TContext> *ptr) { this->_ptrObserver = ptr; }

protected:
  void dispatchObservation(TContext &ctx) {
    if (nullptr == this->_ptrObserver) {
      return;
    }

    this->_ptrObserver->onObservation(ctx);
  }

private:
  Observer<TContext> *_ptrObserver;
};
} // namespace xtrpg::interface