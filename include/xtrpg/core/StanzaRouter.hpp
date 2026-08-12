#pragma once

#include "xtrpg/core/StanzaContext.hpp"
#include "xtrpg/interfaces/IConsumer.hpp"
#include "xtrpg/interfaces/IObserver.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <vector>

using xtrpg::core::StanzaContext;

namespace xtrpg {
namespace core {

class StanzaRouter {
public:
  StanzaRouter();
  ~StanzaRouter();

  // Start worker thread(s)
  void start();

  // Gracefully flush queue and shut down worker thread
  void stop();

  // Async Non-Blocking Enqueue (Called by Boost.Asio network threads)
  void postStanza(StanzaContext ctx);

  // --- Pipeline Registration ---
  void addObserver(std::shared_ptr<IObserver<StanzaContext>> observer);
  void
  removeObserver(const std::shared_ptr<IObserver<StanzaContext>> &observer);

  void addConsumer(std::shared_ptr<IConsumer<StanzaContext>> consumer);
  void
  removeConsumer(const std::shared_ptr<IConsumer<StanzaContext>> &consumer);

private:
  // Background worker loop running on its own thread.
  void workerLoop();

  // Synchronous execution pipeline
  void processStanza(StanzaContext &ctx);

  // Thread Safety & Queue State
  std::queue<StanzaContext> stanzaQueue;

  // Lock for the queue processing
  std::mutex stanzaQueueMutex;

  std::condition_variable stanzaQueueConditionVariable;

  // Boolean indicator to determine if the worker thread is currently running.
  std::atomic<bool> isWorkerRunning{false};

  // Worker thread for processing the stanza queue.
  std::thread workerThread;

  // Registered Listeners and Consumers
  mutable std::shared_mutex stanzaProcessingMutex;
  std::vector<std::shared_ptr<IObserver<StanzaContext>>> observers;
  std::vector<std::shared_ptr<IConsumer<StanzaContext>>> consumers;
};

} // namespace core
} // namespace xtrpg
