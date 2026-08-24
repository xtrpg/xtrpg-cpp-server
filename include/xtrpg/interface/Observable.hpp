#pragma once

#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include "xtrpg/interface/Observer.hpp"

namespace xtrpg::interface {
template <typename TContext> class Observable {
public:
  virtual ~Observable() = default;

  void addObserver(const std::shared_ptr<Observer<TContext>> &ptr) {
    if (ptr) {
      this->_observers.push_back(ptr);
    }
  }

  void eraseObserver(const Observer<TContext> *pTarget) {
    if (!pTarget) {
      return;
    }

    std::erase_if(this->_observers,
                  [pTarget](const std::weak_ptr<Observer<TContext>> &wp) {
                    auto sp = wp.lock();
                    return !sp || sp.get() == pTarget;
                  });
  }

  void clearObservers() { this->_observers.clear(); }

protected:
  void dispatchObservation(TContext &ctx) {
    std::erase_if(this->_observers,
                  [&ctx](const std::weak_ptr<Observer<TContext>> &wp) {
                    // Attempt to gain temporary ownership of the current
                    // observer.
                    if (auto observer = wp.lock()) {
                      // Dispatch the observation.
                      observer->onObservation(ctx);

                      // Pointer is still valid, keep it in the vector.
                      return false;
                    }

                    // Pointer has expired, remove it from the observers vector.
                    return true;
                  });
  }

private:
  std::vector<std::weak_ptr<Observer<TContext>>> _observers;
};
} // namespace xtrpg