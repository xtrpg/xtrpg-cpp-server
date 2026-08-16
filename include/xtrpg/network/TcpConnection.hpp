#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <iostream>
#include <memory>
#include <optional>

namespace xtrpg::network {

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {

public:
  explicit TcpConnection(asio::ip::tcp::socket tcpSocket)
      : _tcpSocket(std::move(tcpSocket)) {}

  void upgradeToTls(asio::ssl::context &ssl_ctx);

  void write(std::string_view data);

  TcpConnection &operator<<(std::string_view str) {
    this->write(str);
    return *this;
  }

private:
  bool _isTlsActive{false};
  asio::ip::tcp::socket _tcpSocket;
  std::optional<asio::ssl::stream<asio::ip::tcp::socket>> _sslStream;
};
} // namespace xtrpg::network