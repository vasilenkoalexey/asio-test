#ifndef H2SERVER_H
#define H2SERVER_H

#include <set>

#include "asio.hpp"
#include "cbor11.h"
#include "message_header.h"


namespace network {
class Server {
public:
  Server(asio::io_context &io_context, const asio::ip::tcp::endpoint &endpoint)
      : acceptor(io_context, endpoint), context(io_context) {
    accept();
  }
  void poll() { context.poll(); }
  void broadcast(const uint32_t &opcode, const cbor &item) {
    const cbor::binary msg = cbor::encode(item);
    for (auto session : sessions) {
      session->write_msg(opcode, msg);
    }
  }

private:
  class Session : public std::enable_shared_from_this<Session> {
  public:
    Session(asio::ip::tcp::socket _socket,
            std::set<std::shared_ptr<Session>> &_sessions)
        : socket(std::move(_socket)), sessions(_sessions) {}

  private:
    void start() {
      sessions.insert(shared_from_this());
      read_header();
    }
    void read_header() {
      asio::async_read(socket, asio::buffer(&h, sizeof(h)),
                       [this](std::error_code ec, std::size_t /*length*/) {
                         if (!ec) {
                           if (h.magic == header::MAGIC &&
                               h.length <= b.size()) {
                             read_body();
                           } else {
                             read_header();
                           }
                         } else {
                           sessions.erase(shared_from_this());
                           socket.close();
                         }
                       });
    }
    void read_body() {
      asio::async_read(socket, asio::buffer(b.data(), h.length),
                       [this](std::error_code ec, std::size_t /*length*/) {
                         if (!ec) {
                           read_header();
                         } else {
                           sessions.erase(shared_from_this());
                           socket.close();
                         }
                       });
    }
    void write_msg(const uint32_t &opcode, const cbor::binary &msg) {
      header header_out(opcode, static_cast<uint32_t>(msg.size()));
      std::array<asio::const_buffer, 2> sequence{
          asio::buffer(&header_out, sizeof(header)),
          asio::buffer(msg.data(), msg.size())};
      asio::async_write(socket, sequence,
                        [this](std::error_code ec, std::size_t /*length*/) {
                          if (ec) {
                            sessions.erase(shared_from_this());
                            socket.close();
                          }
                        });
    }

    std::set<std::shared_ptr<Session>> &sessions;
    asio::ip::tcp::socket socket;
    header h;
    std::array<char, 512> b{};

    header oh;
    std::array<char, 512> ob;
    std::array<asio::const_buffer, 2> asiobuf = {
        asio::buffer(&oh, sizeof(oh)), asio::buffer(ob.data(), ob.size())};

    friend class Server;
  };

  void accept() {
    acceptor.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
          if (!ec) {
            std::make_shared<Session>(std::move(socket), sessions)->start();
          }
          accept();
        });
  }
  asio::io_context &context;
  std::set<std::shared_ptr<Session>> sessions;
  asio::ip::tcp::acceptor acceptor;
};
} // namespace network

#endif