#include "xtrpg/network/SocketConnectionListener.hpp"

namespace xtrpg::network {

namespace {
bool isListenerShutdownError(const std::error_code &ec) {
  return ec == asio::error::operation_aborted ||
         ec == asio::error::bad_descriptor;
}
} // namespace

void SocketConnectionListener::initializeAcceptors() {
  try {
    asio::ip::tcp::acceptor ipv6Acceptor(
        *this->_ptrIoContext,
        asio::ip::tcp::endpoint(asio::ip::tcp::v6(), this->_port));

    asio::ip::v6_only option(false);
    ipv6Acceptor.set_option(option);
    this->_ipv6Acceptor.emplace(std::move(ipv6Acceptor));

    std::cout
        << "[SocketConnectionListener] Enabled IPv6 dual-stack listener on "
        << this->_port << std::endl;
    return;
  } catch (const std::exception &ex) {
    std::cerr << "[SocketConnectionListener] Failed to open IPv6 dual-stack "
                 "socket on "
              << "port " << this->_port << ": " << ex.what() << std::endl;
    this->_ipv6Acceptor.reset();
  }

  try {
    this->_ipv4Acceptor.emplace(
        *this->_ptrIoContext,
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), this->_port));
  } catch (const std::exception &ex) {
    std::cerr
        << "[SocketConnectionListener] Failed to open IPv4 socket on port "
        << this->_port << ": " << ex.what() << std::endl;
    this->_ipv4Acceptor.reset();
  }
}

void SocketConnectionListener::start() {
  if (!this->_isStopped) {
    return;
  }
  this->_isStopped = false;

  std::cout << "[SocketConnectionListener] Listening for socket connections."
            << std::endl;

  if (this->_ipv4Acceptor) {
    std::cout << "                           IPv4 on port "
              << this->_ipv4Acceptor->local_endpoint().port() << "."
              << std::endl;
    this->acceptIPv4Connections();
  }

  if (this->_ipv6Acceptor) {
    std::cout << "                           IPv6 on port "
              << this->_ipv6Acceptor->local_endpoint().port() << "."
              << std::endl;
    this->acceptIPv6Connections();
  }
}

void SocketConnectionListener::stop() {
  if (this->_isStopped)
    return;
  this->_isStopped = true;

  std::error_code ec;

  if (this->_ipv4Acceptor) {
    this->_ipv4Acceptor->cancel(ec);
    this->_ipv4Acceptor->close(ec);
  }

  if (this->_ipv6Acceptor) {
    this->_ipv6Acceptor->cancel(ec);
    this->_ipv6Acceptor->close(ec);
  }

  std::cout << "[SocketConnectionListener] Stopped listening for socket "
               "connections."
            << std::endl;
}

void SocketConnectionListener::acceptIPv4Connections() {
  if (!this->_ipv4Acceptor) {
    return;
  }

  this->_ipv4Acceptor->async_accept([this](std::error_code ec,
                                           asio::ip::tcp::socket socket) {
    if (!ec) {
      std::cout << "[SocketConnectionListener] New incoming IPv4 connection."
                << std::endl;

      TcpConnection *ptrTcpConnection = new TcpConnection(socket);
      if (!this->dispatchObservation(ptrTcpConnection)) {
        delete ptrTcpConnection;
      }
    } else if (!isListenerShutdownError(ec)) {
      std::cerr << "[SocketConnectionListener] IPv4 accept failed: "
                << ec.message() << std::endl;
    }

    if (!this->_isStopped && !isListenerShutdownError(ec)) {
      this->acceptIPv4Connections();
    }
  });
}

void SocketConnectionListener::acceptIPv6Connections() {
  if (!this->_ipv6Acceptor) {
    return;
  }

  this->_ipv6Acceptor->async_accept([this](std::error_code ec,
                                           asio::ip::tcp::socket socket) {
    if (!ec) {
      std::cout << "[SocketConnectionListener] New incoming IPv6 connection."
                << std::endl;

      TcpConnection *ptrTcpConnection = new TcpConnection(socket);
      if (!this->dispatchObservation(ptrTcpConnection)) {
        delete ptrTcpConnection;
      }
    } else if (!isListenerShutdownError(ec)) {
      std::cerr << "[SocketConnectionListener] IPv6 accept failed: "
                << ec.message() << std::endl;
    }

    if (!this->_isStopped && !isListenerShutdownError(ec)) {
      this->acceptIPv6Connections();
    }
  });
}
} // namespace xtrpg::network