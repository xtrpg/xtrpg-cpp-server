#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>
#include <signal.h>
#include <thread>
#include <vector>

#include <asio.hpp>

#include "version.hpp"
#include "xtrpg/config/ConfigManager.hpp"
#include "xtrpg/interface/Observer.hpp"
#include "xtrpg/network/SocketConnectionListener.hpp"
#include "xtrpg/xml/tokenizer/XmlStreamTokenizer.hpp"
#include "xtrpg/xmpp/ClientConnectionManager.hpp"
#include "xtrpg/xmpp/session/ClientSession.hpp"

#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>
#endif

// Global flag for signal handling
std::atomic<bool> g_shouldShutdown{false};

// Signal handler for graceful shutdown
void signalHandler(int signal) {
  std::cout << "\nReceived signal " << signal
            << ", initiating graceful shutdown..." << std::endl;
  g_shouldShutdown = true;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
  // Set console codepages to UTF-8 (65001)
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif

  try {

    // Output preamble information about the application and its configuration
    // system.
    std::cout << "XTRPG: A XMPP Server" << std::endl
              << "Version: " << xtrpg::VERSION << std::endl
              << "Copyright (C) 2026 XTRPG Contributors" << std::endl
              << "License: MIT" << std::endl
              << "         "
                 "<https://github.com/xtrpg/xtrpg-cpp-server/blob/main/LICENSE>"
              << std::endl
              << std::endl
              << "This is free and open-source software." << std::endl
              << "You are free to use, modify, and redistribute it under the "
                 "terms of the MIT License."
              << std::endl
              << "There is NO WARRANTY for this software." << std::endl
              << std::endl;

    // Discover and register all modules' configuration schemas, then load the
    // configuration file and parse command-line arguments.
    xtrpg::config::ConfigManager configManager;
    configManager.registerAllDiscoveredModules();
    std::ifstream configFile("./config.toml");
    if (configFile) {
      configManager.loadTomlFile(configFile);
    }
    configManager.parseCLI(argc, argv);

    // Initialize Asio IO context for async I/O operations
    asio::io_context ioContext;

    // Get the port from configuration (default 5222 for XMPP C2S)
    auto portValue = configManager.get<int64_t>("c2s", "port").value_or(5222);
    uint16_t listeningPort = static_cast<uint16_t>(portValue);

    // Create temporary connection handler to listen for incoming connections
    // and create ClientSession instances
    xtrpg::xmpp::ClientConnectionManager connectionManager(ioContext,
                                                           listeningPort);

    // Set up signal handlers for graceful shutdown (SIGINT and SIGTERM)
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
#ifndef _WIN32
    signal(SIGHUP, signalHandler);
#endif

    std::cout << "[INFO] Server running. Press Ctrl+C to shutdown."
              << std::endl;

    // Run IO context in a thread pool for handling async operations
    std::vector<std::thread> ioThreads;
    const size_t threadCount = std::thread::hardware_concurrency();
    for (size_t i = 0; i < threadCount; ++i) {
      ioThreads.emplace_back([&ioContext]() { ioContext.run(); });
    }

    // Main thread: wait for shutdown signal
    int previousCount = -1;
    while (!g_shouldShutdown) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (previousCount != connectionManager.countConnections()) {
        std::cout << "[main] Connection Count: "
                  << connectionManager.countConnections() << std::endl;
        previousCount = connectionManager.countConnections();
      }
    }

    std::cout << "[INFO] Shutting down IO context..." << std::endl;
    ioContext.stop();

    // Wait for all IO threads to complete
    for (auto &thread : ioThreads) {
      if (thread.joinable()) {
        thread.join();
      }
    }

    std::cout << "[INFO] Server shutdown complete." << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION OCCURRED" << std::endl
              << "Application closing die to \"" << e.what() << "\"."
              << std ::endl;
    return 1;
  } catch (...) {
    std::cerr << "UNEXPECTED EXCEPTION OCCURRED" << std::endl
              << "Application closing." << std ::endl;
    return 1;
  }

  return 0;
}