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

/// This Queue is intended for SocketCommunication to push requests which should be sent onto it.
/// It ensures that the invocations of asio::aSend are done serially.
class SendQueue 
{
  public:

    using TCP           = boost::asio::ip::tcp;
    using SocketService = boost::asio::stream_socket_service<TCP>;
    using Socket   = boost::asio::basic_stream_socket<TCP, SocketService>;

    void push(std::shared_ptr<Socket> sock, boost::asio::const_buffers_1 data, std::function<void()> callback);

  private:

    void dispatch();

    struct SendItem
    {
      std::shared_ptr<Socket> sock;
      boost::asio::const_buffers_1 data;
      std::function<void()> callback;
    };

    std::deque<SendItem> _itemQueue;
    std::mutex _sendMutex;
    bool _ready = true;
};

}}
#endif // not PRECICE_NO_SOCKETS
