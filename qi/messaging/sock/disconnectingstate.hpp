#pragma once
#ifndef _QI_SOCK_DISCONNECTINGSTATE_HPP
#define _QI_SOCK_DISCONNECTINGSTATE_HPP
#include <qi/future.hpp>
#include <qi/messaging/sock/common.hpp>
#include <qi/messaging/sock/macrolog.hpp>
#include <qi/messaging/sock/traits.hpp>

namespace qi
{
  namespace sock
  {
    /// Disconnecting state of the socket.
    /// Close the socket immediately.
    ///
    /// The disconnected promise is stored only to be passed to the next state.
    /// It is not the responsibility of Disconnecting to set it.
    ///
    /// Network N,
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    class Disconnecting
    {
      SocketPtr<S> _socket;
      Promise<void> _disconnectedPromise;
      Promise<void> _completePromise;
    public:
      Disconnecting(const SocketPtr<S>& socket, Promise<void> disconnectedPromise)
        : _socket(socket)
        , _disconnectedPromise(disconnectedPromise)
      {
        QI_LOG_DEBUG_SOCKET(_socket.get()) << "Entering Disconnecting state";
        // The socket is the output of the connecting state. It this state fails,
        // we don't have any socket, so it is null here.
        // Even in this case, we still enter the disconnecting state for
        // consistency reasons.
        if (socket)
        {
          auto completePromise = _completePromise;
          socket->get_io_service().wrap([=]() mutable {
            QI_LOG_DEBUG_SOCKET(socket.get()) << "Disconnecting: before socket close";
            close<N>(socket);
            completePromise.setValue(nullptr);
          })();
        }
        else
        {
          // Nothing to do: disconnection is therefore complete.
          _completePromise.setValue(nullptr);
        }
      }
      // TODO: replace by "= default" when get rid of VS2013.
      Disconnecting(Disconnecting&& x)
        : _socket(std::move(x._socket))
        , _disconnectedPromise(std::move(x._disconnectedPromise))
        , _completePromise(std::move(x._completePromise))
      {
      }
      // TODO: replace by "= default" when get rid of VS2013.
      Disconnecting& operator=(Disconnecting&& x)
      {
        _socket = std::move(x._socket);
        _disconnectedPromise = std::move(x._disconnectedPromise);
        _completePromise = std::move(x._completePromise);
        return *this;
      }
      Future<void> disconnectedPromise() const
      {
        return _disconnectedPromise.future();
      }
      Future<void> complete() const
      {
        QI_LOG_DEBUG_SOCKET(_socket.get()) << "Disconnecting: asking the 'complete' future";
        return _completePromise.future();
      }
    };
}} // namespace qi::sock

#endif // _QI_SOCK_DISCONNECTINGSTATE_HPP
