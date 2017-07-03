#include <stdexcept>
#include <thread>
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>
#include <qi/messaging/sock/accept.hpp>
#include <qi/messaging/sock/connect.hpp>
#include <qi/messaging/sock/networkasio.hpp>
#include "src/messaging/tcpmessagesocket.hpp"
#include <qi/future.hpp>
#include <qi/scoped.hpp>
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
  auto _ = scopedSetAndRestore(
    N::acceptor_type::async_accept,
    mock::defaultAsyncAccept
  );

  Promise<SocketPtr<N>> promiseAcceptFinished;
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  Endpoint<Lowest<SslSocket<N>>> endpoint;
  AcceptConnectionContinuous<N> accept{N::defaultIoService()};
  accept(context, endpoint, ReuseAddressEnabled{true},
    [&](ErrorCode<N> e, SocketPtr<N> s) {
      if (e) promiseAcceptFinished.setError("Accept error occurred.");
      else   promiseAcceptFinished.setValue(s);
      return false;
    }
  );
  auto fut = promiseAcceptFinished.future();
  ASSERT_EQ(FutureState_FinishedWithValue, fut.waitFor(defaultTimeout));
  ASSERT_TRUE(fut.value());
}

TEST(NetAcceptConnectionContinuous, AcceptFailed)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  auto _ = scopedSetAndRestore(
    N::acceptor_type::async_accept,
    [](SslSocket<N>::next_layer_type&, N::_anyHandler h) {
      h(networkUnreachable<ErrorCode<N>>());
    }
  );

  Promise<SocketPtr<N>> promiseAcceptFinished;
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  Endpoint<Lowest<SslSocket<N>>> endpoint;
  AcceptConnectionContinuous<N> accept{N::defaultIoService()};
  accept(context, endpoint,
    ReuseAddressEnabled{false},
    [&](ErrorCode<N> e, SocketPtr<N> s) {
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

  auto& io = N::defaultIoService();
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  const Url url{"tcp://127.0.0.1:51234"};
  Promise<SocketPtr<N>> promiseAcceptFinished;

  AcceptConnectionContinuous<N> accept{io};
  accept(context, url,
    IpV6Enabled{false}, ReuseAddressEnabled{false},
    [&](ErrorCode<N> erc, SocketPtr<N> socket) {
      if (erc)
      {
        throw std::runtime_error{std::string{"Accept error: "} + erc.message()};
      }
      promiseAcceptFinished.setValue(socket);
      return false;
    }
  );
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectSocketFuture<N> connect{io};
  connect(url, SslEnabled{false}, context, IpV6Enabled{false}, Side::client);
  ASSERT_EQ(FutureState_FinishedWithValue, connect.complete().waitFor(defaultTimeout));

  auto fut = promiseAcceptFinished.future();
  ASSERT_EQ(FutureState_FinishedWithValue, fut.waitFor(defaultTimeout));
  ASSERT_TRUE(fut.value());
}
