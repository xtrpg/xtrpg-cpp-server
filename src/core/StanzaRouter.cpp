#include "xtrpg/core/StanzaRouter.hpp"
#include <iostream>

using xtrpg::core::StanzaContext;
using xtrpg::core::StanzaRouter;

/**
 * Default Constructor.
 */
StanzaRouter::StanzaRouter() = default;

/**
 * Destructor.
 */
StanzaRouter::~StanzaRouter() { this->stop(); }

/**
 * Starts the background worker on it's own thread.
 */
void StanzaRouter::start() {
  if (this->isWorkerRunning)
    return;

  this->isWorkerRunning = true;
  workerThread = std::thread(&StanzaRouter::workerLoop, this);
}

/**
 * Stops the current running background worker.
 */
void StanzaRouter::stop() {
  if (!this->isWorkerRunning)
    return;

  {
    std::lock_guard lock(this->stanzaQueueMutex);
    this->isWorkerRunning = false;
  }
  this->stanzaQueueConditionVariable.notify_all();

  if (this->workerThread.joinable()) {
    this->workerThread.join();
  }
}

/**
 * Submits a new stanza to be queued for processing.
 */
void StanzaRouter::postStanza(StanzaContext ctx) {
  {
    std::lock_guard lock(this->stanzaQueueMutex);
    this->stanzaQueue.push(std::move(ctx));
  }
  this->stanzaQueueConditionVariable
      .notify_one(); // Wake up background worker thread
}

/**
 * Work loop for the child thread.
 */
void StanzaRouter::workerLoop() {
  while (this->isWorkerRunning) {
    StanzaContext ctx;

    {
      std::unique_lock lock(this->stanzaQueueMutex);

      // Sleep worker thread until there is a stanza in the queue or stop() is
      // called
      this->stanzaQueueConditionVariable.wait(lock, [this] {
        return !this->stanzaQueue.empty() || !this->isWorkerRunning;
      });

      if (!this->isWorkerRunning && this->stanzaQueue.empty()) {
        break;
      }

      ctx = std::move(this->stanzaQueue.front());
      this->stanzaQueue.pop();
    }

    // Process stanza on dedicated worker thread
    processStanza(ctx);
  }
}

/**
 * Processes a single stanza.
 */
void StanzaRouter::processStanza(StanzaContext &ctx) {
  std::shared_lock lock(this->stanzaProcessingMutex);

  // Run Observers
  for (const auto &observer : this->observers) {

    try {
      observer->onObservation(ctx);
    } catch (const std::exception &e) {
      std::cerr << "[Router Worker] Observer error " << ": " << e.what()
                << std::endl;
    }
  }

  // Run Consumers (Feature Modules -> C2S Sessions -> S2S Outbound)
  for (const auto &consumer : this->consumers) {
    if (consumer->canConsume(ctx)) {
      try {
        if (consumer->consume(ctx)) {
          ctx.handled = true;
          return; // Stanza consumed! Short-circuit remaining consumers.
        }
      } catch (const std::exception &e) {
        std::cerr << "[Router Worker] Consumer error in " << consumer->getName()
                  << ": " << e.what() << std::endl;
      }
    }
  }
}

void StanzaRouter::addObserver(
    std::shared_ptr<IObserver<StanzaContext>> observer) {
  std::unique_lock lock(this->stanzaProcessingMutex);
  this->observers.push_back(std::move(observer));
}

void StanzaRouter::removeObserver(
    const std::shared_ptr<IObserver<StanzaContext>> &observer) {
  std::unique_lock lock(this->stanzaProcessingMutex);
  std::erase(this->observers, observer);
}

void StanzaRouter::addConsumer(
    std::shared_ptr<IConsumer<StanzaContext>> consumer) {
  std::unique_lock lock(this->stanzaProcessingMutex);
  this->consumers.push_back(std::move(consumer));
}

void StanzaRouter::removeConsumer(
    const std::shared_ptr<IConsumer<StanzaContext>> &consumer) {
  std::unique_lock lock(this->stanzaProcessingMutex);
  std::erase(this->consumers, consumer);
}