#ifndef PRECICE_NO_SOCKETS

#pragma once

#include <deque>
#include <boost/asio.hpp>
#include <functional>
#include <mutex>
namespace precice
{
namespace com
{

class SendQueue 
{
  using TCP           = boost::asio::ip::tcp;
  using SocketService = boost::asio::stream_socket_service<TCP>;
  using Socket   = boost::asio::basic_stream_socket<TCP, SocketService>;
  public:
    void push(std::shared_ptr<Socket> sock, boost::asio::const_buffers_1 data, std::function<void()> callback);
  private:
    void process();
    struct SendItem
    {
      std::shared_ptr<Socket> sock;
      boost::asio::const_buffers_1 data;
      std::function<void()> callback;
    };
    std::deque<SendItem> _itemQueue;
    std::mutex _queueMutex;
};

}}
#endif // not PRECICE_NO_SOCKETS
