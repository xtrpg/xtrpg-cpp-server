#pragma once

#include <asio.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include "xtrpg/interfaces/IObservable.hpp"
#include "xtrpg/network/TcpConnection.hpp"

namespace xtrpg::network {
class SocketConnectionListener
    : public IObservable<std::shared_ptr<TcpConnection>> {
public:
  /**
   * Instantiates a new listener instance.
   */
  SocketConnectionListener(asio::io_context &ioContext, uint16_t port)
      : _ioContext(ioContext),
        _ipv4Acceptor(ioContext,
                      asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
        _ipv6Acceptor(ioContext,
                      asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port)) {};

  void start();
  void stop();

private:
  void acceptIPv4Connections();
  void acceptIPv6Connections();

  asio::io_context &_ioContext;
  asio::ip::tcp::acceptor _ipv4Acceptor;
  asio::ip::tcp::acceptor _ipv6Acceptor;
  bool _isStopped{true};
};
} // namespace xtrpg::network