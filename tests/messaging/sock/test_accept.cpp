#include <stdexcept>
#include <thread>
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>
#include <src/messaging/sock/accept.hpp>
#include <src/messaging/sock/connect.hpp>
#include <src/messaging/sock/networkasio.hpp>
#include <src/messaging/sock/sslcontextptr.hpp>
#include "src/messaging/tcpmessagesocket.hpp"
#include <qi/future.hpp>
#include <ka/scoped.hpp>
#include <qi/url.hpp>
#include "networkmock.hpp"
#include "networkcommon.hpp"

static const qi::MilliSeconds defaultTimeout{500};

////////////////////////////////////////////////////////////////////////////////
// NetAcceptConnectionContinuous tests:
////////////////////////////////////////////////////////////////////////////////

TEST(NetAcceptConnectionContinuous, Success)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  auto _ = ka::scoped_set_and_restore(
    Acceptor<N>::async_accept,
    mock::defaultAsyncAccept
  );

  auto& io = N::defaultIoService();
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  Promise<SocketPtr<S>> promiseAcceptFinished;
  Endpoint<Lowest<S>> endpoint;

  AcceptConnectionContinuous<N, S> accept{io};
  accept([&]{ return makeSslSocketPtr<N>(io, context); }, endpoint, ReuseAddressEnabled{true},
    [&](ErrorCode<N> e, SocketPtr<S> s) {
      if (e) promiseAcceptFinished.setError("Accept error occurred.");
      else   promiseAcceptFinished.setValue(s);
      return false;
    }
  );
  auto fut = promiseAcceptFinished.future();
  ASSERT_EQ(FutureState_FinishedWithValue, fut.waitFor(defaultTimeout)) << fut.error();
  ASSERT_TRUE(fut.value());
}

TEST(NetAcceptConnectionContinuous, AcceptFailed)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  auto _ = ka::scoped_set_and_restore(
    Acceptor<N>::async_accept,
    [](S::next_layer_type&, N::_anyHandler h) {
      h(networkUnreachable<ErrorCode<N>>());
    }
  );

  auto& io = N::defaultIoService();
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  Promise<SocketPtr<S>> promiseAcceptFinished;
  Endpoint<Lowest<S>> endpoint;

  AcceptConnectionContinuous<N, S> accept{io};
  accept([&]{ return makeSslSocketPtr<N>(io, context); }, endpoint,
    ReuseAddressEnabled{false},
    [&](ErrorCode<N> e, SocketPtr<S> s) {
      if (e) promiseAcceptFinished.setError(e.message());
      else promiseAcceptFinished.setValue(s);
      return false;
    }
  );
  auto fut = promiseAcceptFinished.future();
  ASSERT_EQ(FutureState_FinishedWithError, fut.waitFor(defaultTimeout));
  ASSERT_EQ("networkUnreachable", fut.error());
}

TEST(NetAcceptConnectionContinuous, SuccessWithListenAsio)
{
  using namespace qi;
  using namespace qi::sock;
  using N = NetworkAsio;
  using S = SslSocket<N>;
  using E = Endpoint<Lowest<S>>;

  auto& io = N::defaultIoService();
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  Promise<SocketPtr<S>> promiseAcceptFinished;
  Promise<E> localEndpoint;

  auto makeSocket = [&]{ return makeSslSocketPtr<N>(io, context); };
  AcceptConnectionContinuous<N, S> accept{io};
  accept(makeSocket, "tcp://127.0.0.1:0",
    IpV6Enabled{false}, ReuseAddressEnabled{false},
    [&](ErrorCode<N> erc, SocketPtr<S> socket) { // onAccept
      if (erc)
      {
        promiseAcceptFinished.setError(erc.message());
        throw std::runtime_error{std::string{"Accept error: "} + erc.message()};
      }
      promiseAcceptFinished.setValue(socket);
      return false;
    },
    [&](ErrorCode<N> erc, boost::optional<E> ep) { // onListen
      if (erc)
      {
        localEndpoint.setError(erc.message());
        throw std::runtime_error{std::string{"Listen error: "} + erc.message()};
      }
      if (!ep)
      {
        localEndpoint.setError("Local endpoint is undefined");
        throw std::runtime_error{std::string{"Listen error: local endpoint is undefined"}};
      }
      localEndpoint.setValue(*ep);
    }
  );
  using Side = HandshakeSide<S>;
  ConnectSocketFuture<N, S> connect{io};
  connect(url(localEndpoint.future().value(), TcpScheme::Raw), makeSocket,
          IpV6Enabled{false}, Side::client);
  ASSERT_EQ(FutureState_FinishedWithValue, connect.complete().waitFor(defaultTimeout)) << connect.complete().error();

  auto fut = promiseAcceptFinished.future();
  ASSERT_EQ(FutureState_FinishedWithValue, fut.waitFor(defaultTimeout));
  ASSERT_TRUE(fut.value());
}
