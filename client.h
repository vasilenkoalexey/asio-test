#ifndef H2CLIENT_H
#define H2CLIENT_H

#include <cstdlib>
#include <deque>
#include <iostream>

#include "asio.hpp"
#include "message_header.h"

namespace network {
class Client {
public:
  Client(asio::io_context &io_context,
         const asio::ip::tcp::resolver::results_type &endpoints,
         std::function<void(const uint32_t, const std::string &)> _callback)
      : io_context(io_context), socket(io_context), callback(_callback) {
    asio::async_connect(socket, endpoints,
                        [this](std::error_code ec, asio::ip::tcp::endpoint) {
                          if (!ec) {
                            std::cout << "Connected \n";
                            read_header();
                          } else {
                            std::cerr << ec.message() << "\n";
                          }
                        });
  }

  void write_msg(const uint32_t &opcode, const std::string &msg) {
      header header_out(opcode, static_cast<uint32_t>(msg.size()));
      std::array<asio::const_buffer, 2> sequence{
          asio::buffer(&header_out, sizeof(header)),
          asio::buffer(msg.data(), msg.size())};
    asio::async_write(socket, sequence,
                      [this](std::error_code ec, std::size_t /*length*/) {
                        if (ec) {
                          socket.close();
                        }
                      });
  }
  void poll() { io_context.poll(); }

private:
  void read_header() {
    asio::async_read(socket,
                     asio::buffer(&ih, sizeof(ih)),
                     [this](std::error_code ec, std::size_t /*length*/) {
                       if (!ec) {
                         if (ih.magic == header::MAGIC &&
                             ih.length <= ib.size()) {
                           read_body();
                         }
                       } else {
                         socket.close();
                       }
                     });
  }
  void read_body() {
    asio::async_read(
        socket, asio::buffer(ib.data(), ih.length),
        [this](std::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            callback(ih.opcode,
                     std::string(ib.data(), ih.length));
            read_header();
          } else {
            socket.close();
          }
        });
  }
  std::function<void(const uint32_t, const std::string &)> callback;
  asio::io_context &io_context;
  asio::ip::tcp::socket socket;
  header ih;
  std::array<char, 512> ib;
//  header oh;
//  std::array<char, 512> ob;
//  std::array<asio::const_buffer, 2> asiobuf = { asio::buffer(&oh, sizeof(oh)), asio::buffer(ob.data(), ob.size()) };
};
} // namespace network

#endif