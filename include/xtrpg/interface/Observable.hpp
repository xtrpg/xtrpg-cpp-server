#pragma once

#include <iostream>
#include <memory>
#include <mutex>
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

  void setObserver(Observer<TContext> *ptr) {
    std::lock_guard lock(this->_observerMutex);
    this->_ptrObserver = ptr;
  }

protected:
  bool dispatchObservation(TContext &ctx) {
    std::lock_guard lock(this->_observerMutex);
    if (nullptr == this->_ptrObserver) {
      return false;
    }

    this->_ptrObserver->onObservation(ctx);
    return true;
  }

private:
  Observer<TContext> *_ptrObserver = nullptr;
  std::mutex _observerMutex;
};
} // namespace xtrpg::interface