#include <string>
#include <algorithm>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>
#include <src/messaging/sock/resolve.hpp>
#include <ka/src.hpp>
#include <src/messaging/sock/connect.hpp>
#include <src/messaging/sock/networkasio.hpp>
#include <ka/scoped.hpp>
#include <qi/future.hpp>
#include "networkmock.hpp"
#include "networkcommon.hpp"

static const qi::MilliSeconds defaultTimeout{500};

template<typename T>
struct NetworkType
{
  using type = typename T::Network;
};

template<typename T>
using Network = typename NetworkType<T>::type;

namespace qi { namespace sock {

template<typename N>
struct ResolveUrlListFun
{
  using Network = N;
  ErrorCode<N> operator()(IoService<N>& io, const Url& url) const
  {
    Promise<ErrorCode<N>> promise;
    ResolveUrlList<N> resolve{io};
    resolve(url,
      [=](ErrorCode<N> err, Iterator<Resolver<N>>) mutable {
        promise.setValue(err);
      }
    );
    return promise.future().value();
  }
};

template<typename N>
struct ResolveUrlFun
{
  using Network = N;
  ErrorCode<N> operator()(IoService<N>& io, const Url& url) const
  {
    Promise<ErrorCode<N>> promise;
    ResolveUrl<N> resolve{io};
    resolve(url, IpV6Enabled{false},
      [=](ErrorCode<N> err, boost::optional<Entry<Resolver<N>>>) mutable {
        promise.setValue(err);
      }
    );
    return promise.future().value();
  }
};

template<typename N>
struct ConnectSocketFun
{
  using Network = N;
  ErrorCode<N> operator()(IoService<N>& io, const Url& url) const
  {
    Promise<ErrorCode<N>> promise;
    ConnectSocket<N, SslSocket<N>> connect{io};
    SslContext<N> context { Method<SslContext<N>>::tlsv12 };
    connect(url, [&]{ return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{false},
      HandshakeSide<SslSocket<N>>::client,
      [=](ErrorCode<N> err, SslSocketPtr<N>) mutable {
        promise.setValue(err);
      }
    );
    return promise.future().value();
  }
};

template<typename N>
struct ConnectSocketFutureFun
{
  using Network = N;
  ErrorCode<N> operator()(IoService<N>& io, const Url& url) const
  {
    ConnectSocketFuture<N, SslSocket<N>> connect{io};
    SslContext<N> context { Method<SslContext<N>>::tlsv12 };
    connect(url, [&] { return makeSslSocketPtr<N>(io, context); },
            IpV6Enabled{ false }, HandshakeSide<SslSocket<N>>::client);
    return stringToError(connect.complete().error());
  }
  ErrorCode<N> stringToError(const std::string& s) const
  {
    const auto e = badAddress<ErrorCode<N>>();
    if (code(s) == e.value()) return e;
    throw std::runtime_error("stringToError: unknown error, detail=" + s);
  }
};

}} // qi::sock


template<typename T>
struct NetResolveUrl : testing::Test
{
};

using sequences = testing::Types<
  // Mock
  qi::sock::ResolveUrlListFun<mock::N>, qi::sock::ResolveUrlFun<mock::N>,
  qi::sock::ConnectSocketFun<mock::N>, qi::sock::ConnectSocketFutureFun<mock::N>,
  // Asio
  qi::sock::ResolveUrlListFun<qi::sock::NetworkAsio>, qi::sock::ResolveUrlFun<qi::sock::NetworkAsio>,
  qi::sock::ConnectSocketFun<qi::sock::NetworkAsio>, qi::sock::ConnectSocketFutureFun<qi::sock::NetworkAsio>
>;

TYPED_TEST_SUITE(NetResolveUrl, sequences);

TYPED_TEST(NetResolveUrl, WrongUrl)
{
  using namespace qi;
  using namespace qi::sock;
  using F = TypeParam;
  using N = Network<F>;
  IoService<N>& io = N::defaultIoService();
  {
    const auto error = F{}(io, Url{});
    ASSERT_EQ(badAddress<ErrorCode<N>>(), error);
  }
  {
    const auto error = F{}(io, Url{"abcd"});
    ASSERT_EQ(badAddress<ErrorCode<N>>(), error);
  }
  {
    const auto error = F{}(io, Url{"10.12.14.15.16"});
    ASSERT_EQ(badAddress<ErrorCode<N>>(), error);
  }
  {
    const auto error = F{}(io, Url{"tcp://10.12.14.15"});
    ASSERT_EQ(badAddress<ErrorCode<N>>(), error);
  }
  {
    const auto error = F{}(io, Url{"tcp://10.12.14.15:-45"});
    ASSERT_EQ(badAddress<ErrorCode<N>>(), error);
  }
  {
    const auto error = F{}(io, Url{"tcp://10.12.14.15:123456"});
    ASSERT_EQ(badAddress<ErrorCode<N>>(), error);
  }
  {
    const auto error = F{}(io, Url{"tcp://10.12.14.15:abcd"});
    ASSERT_EQ(badAddress<ErrorCode<N>>(), error);
  }
}

TEST(NetFindFirstValidIfAny, Ok)
{
  using namespace qi;
  using namespace qi::sock;
  using sock::detail::findFirstValidIfAny;
  using N = mock::Network;
  using Entry = N::_resolver_entry;
  auto entry = [](bool v6, std::string host) {
    return Entry{{{v6, std::move(host)}}};
  };
  auto v4_0 = entry(false, "10.11.12.13");
  auto v4_1 = entry(false, "10.11.12.14");
  auto v6_0 = entry(true, "10.11.12.15");
  using I = Iterator<Resolver<N>>;
  {
    Entry* a[] = {nullptr};
    auto optionalEntry = findFirstValidIfAny(I{a}, I{}, IpV6Enabled{false});
    ASSERT_FALSE(optionalEntry);
  }
  {
    Entry* a[] = {&v4_0, &v4_1, &v6_0, nullptr};
    auto optionalEntry = findFirstValidIfAny(I{a}, I{}, IpV6Enabled{false});
    ASSERT_EQ(v4_0, optionalEntry.value());
    optionalEntry = findFirstValidIfAny(I{a}, I{}, IpV6Enabled{true});
    ASSERT_EQ(v4_0, optionalEntry);
  }
  {
    Entry* a[] = {&v6_0, &v4_0, &v4_1, nullptr};
    auto optionalEntry = findFirstValidIfAny(I{a}, I{}, IpV6Enabled{false});
    ASSERT_EQ(v4_0, optionalEntry.value());
    optionalEntry = findFirstValidIfAny(I{a}, I{}, IpV6Enabled{true});
    ASSERT_EQ(v6_0, optionalEntry.value());
  }
  {
    Entry* a[] = {&v6_0, nullptr};
    auto optionalEntry = findFirstValidIfAny(I{a}, I{}, IpV6Enabled{true});
    ASSERT_EQ(v6_0, optionalEntry.value());
    optionalEntry = findFirstValidIfAny(I{a}, I{}, IpV6Enabled{false});
    ASSERT_FALSE(optionalEntry);
  }
}

TEST(NetResolveUrlList, Success)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  auto _ = ka::scoped_set_and_restore(Resolver<N>::async_resolve, mock::defaultAsyncResolve);
  using I = Iterator<Resolver<N>>;
  Promise<std::pair<ErrorCode<N>, I>> promiseResult;
  IoService<N> io;
  const std::string host = "10.11.12.13";
  ResolveUrlList<N> resolve{io};
  resolve(Url{"tcp://" + host + ":1234"},
    [&](ErrorCode<N> e, I it) mutable {
      promiseResult.setValue({e, it});
    }
  );
  auto fut = promiseResult.future();
  ASSERT_EQ(FutureState_FinishedWithValue, fut.waitFor(defaultTimeout));
  ASSERT_EQ(success<ErrorCode<N>>(), fut.value().first);
  auto it = fut.value().second;
  const N::_resolver_entry entryIpV4{{{false, host}}};
  const N::_resolver_entry entryIpV6{{{true, host}}};
  ASSERT_EQ(entryIpV4, *it);
  ++it;
  ASSERT_EQ(entryIpV6, *it);
}

// Checks that the setup-cancel procedure is called.
//
// We block in the resolve and the only way to unblock is to perform a cancel.
// To perform a cancel, the setup-cancel procedure must have been called.
TEST(NetResolveUrlList, Cancel)
{
  //using Resolve = TypeParam;
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using I = Iterator<Resolver<N>>;

  Promise<std::pair<ErrorCode<N>, I>> promiseResolve;
  std::thread threadResolve;
  auto _ = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    [&](Resolver<N>::query, Resolver<N>::_anyResolveHandler h) {
      threadResolve = std::thread{[=]() mutable {
        // Block until the resolve promise has been set.
        auto p = promiseResolve.future().value();
        h(p.first, p.second);
      }};
    }
  );
  Promise<std::pair<ErrorCode<N>, I>> promiseResult;
  Promise<void> promiseCancel;
  IoService<N> io;
  const std::string host = "10.11.12.13";
  ResolveUrlList<N> resolve{io};
  resolve(
    Url{"tcp://" + host + ":1234"},
    [&](ErrorCode<N> e, I it) { // onComplete
      promiseResult.setValue({e, it});
    },
    [&](Resolver<N>&) { // setupCancel
      promiseCancel.future().andThen([=](void*) mutable {
        promiseResolve.setValue({operationAborted<ErrorCode<N>>(), I{}});
      });
    }
  );
  auto futResult = promiseResult.future();

  // Trigger the cancel.
  promiseCancel.setValue(nullptr);

  // And check that we have a "operation aborted" error.
  ASSERT_EQ(FutureState_FinishedWithValue, futResult.waitFor(defaultTimeout));
  ASSERT_EQ(operationAborted<ErrorCode<N>>(), futResult.value().first);
  threadResolve.join();
}
