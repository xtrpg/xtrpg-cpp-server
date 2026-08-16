#include "xtrpg/network/TcpConnection.hpp"

namespace xtrpg::network {

void TcpConnection::upgradeToTls(asio::ssl::context &ssl_ctx) {
  // Wrap the existing raw socket into Asio SSL stream
  this->_sslStream.emplace(std::move(this->_tcpSocket), ssl_ctx);

  auto self = shared_from_this();
  this->_sslStream->async_handshake(
      asio::ssl::stream_base::server, [this, self](std::error_code ec) {
        if (!ec) {
          this->_isTlsActive = true;
        } else {
          std::cerr << "TLS Handshake Failed: " << ec.message() << "\n";
        }
      });
}

void TcpConnection::write(std::string_view data) {
  if (this->_isTlsActive && this->_sslStream) {
    // Writing to the secure stream;
    asio::async_write(*this->_sslStream, asio::buffer(data),
                      [](std::error_code, std::size_t) {});
    return;
  }

  // Fallback to the raw connection
  asio::async_write(this->_tcpSocket, asio::buffer(data),
                    [](std::error_code, std::size_t) {});
}

} // namespace xtrpg::network