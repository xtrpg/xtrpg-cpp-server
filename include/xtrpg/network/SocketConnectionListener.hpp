#pragma once

#include <asio.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "xtrpg/interface/Observable.hpp"
#include "xtrpg/network/TcpConnection.hpp"

namespace xtrpg::network {
class SocketConnectionListener
    : public interface::Observable<std::shared_ptr<TcpConnection>> {
public:
  /**
   * Instantiates a new listener instance.
   */
  SocketConnectionListener(asio::io_context &ioContext, uint16_t port)
      : _ioContext(ioContext), _port(port), _isStopped(true) {
    this->initializeAcceptors();
  }

  void start();
  void stop();

private:
  void initializeAcceptors();
  void acceptIPv4Connections();
  void acceptIPv6Connections();

  asio::io_context &_ioContext;
  uint16_t _port;
  std::optional<asio::ip::tcp::acceptor> _ipv4Acceptor;
  std::optional<asio::ip::tcp::acceptor> _ipv6Acceptor;
  bool _isStopped{true};
};
} // namespace xtrpg::network