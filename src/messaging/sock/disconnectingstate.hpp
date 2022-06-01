#pragma once
#ifndef _QI_SOCK_DISCONNECTINGSTATE_HPP
#define _QI_SOCK_DISCONNECTINGSTATE_HPP
#include <qi/future.hpp>
#include "common.hpp"
#include "macrolog.hpp"
#include "traits.hpp"

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
      }
      Disconnecting() = default;
      void operator()()
      {
        // The socket is the output of the connecting state. It this state fails,
        // we don't have any socket, so it is null here.
        // Even in this case, we still enter the disconnecting state for
        // consistency reasons.
        if (_socket)
        {
          // We copy members to avoid having a reference to this in the lambda (and having to handle
          // lifetime issues).
          auto completePromise = _completePromise;
          auto socket = _socket;
          auto& io = N::getIoService(*socket);
          io.wrap([=]() mutable {
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
