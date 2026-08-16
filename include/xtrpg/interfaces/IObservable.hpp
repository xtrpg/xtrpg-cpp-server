#pragma once

#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include "xtrpg/interfaces/IObserver.hpp"

namespace xtrpg {
template <typename TContext> class IObservable {
public:
  virtual ~IObservable() = default;

  void addObserver(const std::shared_ptr<IObserver<TContext>> &ptr) {
    if (ptr) {
      this->_observers.push_back(ptr);
    }
  }

  void removeObserver(const std::shared_ptr<IObserver<TContext>> &target) {
    if (!target) {
      return;
    }

    std::erase_if(this->_observers,
                  [&target](const std::weak_ptr<IObserver<TContext>> &wp) {
                    auto sp = wp.lock();
                    return !sp || sp == target;
                  });
  }

  void eraseObserver(const IObserver<TContext> *pTarget) {
    if (!pTarget) {
      return;
    }

    std::erase_if(this->_observers,
                  [pTarget](const std::weak_ptr<IObserver<TContext>> &wp) {
                    auto sp = wp.lock();
                    return !sp || sp.get() == pTarget;
                  });
  }

  void clearObservers() { this->_observers.clear(); }

protected:
  void dispatchObservation(TContext &ctx) {
    std::erase_if(this->_observers,
                  [&ctx](const std::weak_ptr<IObserver<TContext>> &wp) {
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
  std::vector<std::weak_ptr<IObserver<TContext>>> _observers;
};
} // namespace xtrpg