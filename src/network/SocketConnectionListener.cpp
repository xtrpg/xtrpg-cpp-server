#include "xtrpg/network/SocketConnectionListener.hpp"

namespace xtrpg::network {

void SocketConnectionListener::start() {
  if (!this->_isStopped) {
    return;
  }
  this->_isStopped = false;

  std::cout << "[SocketConnectionListener] Listening for socket connections."
            << std::endl
            << "                           IPv4 on port "
            << _ipv4Acceptor.local_endpoint().port() << "." << std::endl
            << "                           IPv6 on port "
            << _ipv6Acceptor.local_endpoint().port() << "." << std::endl;
  this->acceptIPv4Connections();
  this->acceptIPv6Connections();
}

void SocketConnectionListener::stop() {
  if (this->_isStopped)
    return;
  this->_isStopped = true;

  std::error_code ec;
  this->_ipv4Acceptor.close(ec);
  this->_ipv6Acceptor.close(ec);
  std::cout << "[SocketConnectionListener] Stopped listening for socket "
               "connections."
            << std::endl;
  ;
}

void SocketConnectionListener::acceptIPv4Connections() {
  this->_ipv4Acceptor.async_accept([this](std::error_code ec,
                                          asio::ip::tcp::socket socket) {
    if (!ec) {
      std::cout << "[SocketConnectionListener] New incoming IPv4 connection."
                << std::endl;

      auto tcpConnection = std::make_shared<TcpConnection>(std::move(socket));
      this->dispatchObservation(tcpConnection);
    }

    if (!this->_isStopped) {
      this->acceptIPv4Connections();
    }
  });
}

void SocketConnectionListener::acceptIPv6Connections() {
  this->_ipv6Acceptor.async_accept([this](std::error_code ec,
                                          asio::ip::tcp::socket socket) {
    if (!ec) {
      std::cout << "[SocketConnectionListener] New incoming IPv6 connection."
                << std::endl;

      auto tcpConnection = std::make_shared<TcpConnection>(std::move(socket));
      this->dispatchObservation(tcpConnection);
    }

    if (!this->_isStopped) {
      this->acceptIPv6Connections();
    }
  });
}
} // namespace xtrpg::network