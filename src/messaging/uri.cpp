#include <qi/uri.hpp>
#include <qi/assert.hpp>
#include <ka/empty.hpp>
#include <ka/src.hpp>
#include <ka/uri/uri.hpp>
#include <ka/uri/io.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/ip/address.hpp>

namespace qi
{

bool isLoopbackAddress(const std::string& host)
{
  if (host == "localhost")
    return true;
  boost::system::error_code ec;
  const auto addr = boost::asio::ip::address::from_string(host, ec);
  return !ec && addr.is_loopback();
}

ka::opt_t<Uri> uri(const std::string& str) noexcept
{
  using namespace ka;
  auto res = ka::uri(str.begin(), str.end());
  if (empty(res) || iter(res) != str.end())
    return {};
  return opt(src(mv(res)));
}

bool isRelativeEndpoint(const Uri& uri)
{
  return scheme(uri) == uriQiScheme() && uri.authority().empty();
}

const char* uriQiScheme()
{
  return "qi";
}

namespace detail
{
namespace
{
  bool isSdRelativeEndpoint(const Uri& uri)
  {
    return isRelativeEndpoint(uri) && uri.path() == "ServiceDirectory";
  }
  bool hasAuthority(const Uri& uri)
  {
    return !uri.authority().empty();
  }
  /// @pre `hasAuthority(uri)`
  bool hasLoopbackAddress(const Uri& uri)
  {
    return isLoopbackAddress(host(*uri.authority()));
  }

  using UriCmpKey = ka::product_t<int, int, int, Uri>;

  // URI comparison keys allow for a specific total ordering on URI.
  //
  // Algorithm:
  //   - First, URI are partitioned several times according to predicates ('is relative endpoint?',
  //   'is SD?', etc.). These partitions are given a total ordering that can be represented as a
  //   depth-first preorder traversal of the following tree:
  //                                        root
  //                                        ────
  //                                      ╱      ╲
  //                               relEp            !relEp
  //                               ─────            ──────
  //                             ╱       ╲        ╱        ╲
  //                            sd      !sd    auth        !auth
  //                                           ────
  //                                         ╱      ╲
  //                                       lpbk   !lpbk
  //
  //    For instance, the `relEp/sd` partition comes before the `relEp/!sd` partition, which comes
  //    before the `!relEp/auth/lpback` partition, and so on.
  //
  //   - This order on partitions is translated to an order on numerical vectors of dimension 3
  //   (i.e. the greatest depth of the tree). For instance:
  //     + `relEp/sd` -> (0, 0, 0)
  //     + `relEp/!sd` -> (0, 1, 0)
  //     + `!relEp/auth/!lpbk` -> (1, 0, 1)
  //     + `!relEp/!auth` -> (1, 1, 0)
  //   `(0, 0, 0)` comes before `(0, 1, 0)`, which comes before `(1, 0, 1)`, etc., translating the
  //   order on partitions.
  //
  //   - Finally, since we are interested in an order on URI and not on partitions, we need to
  //   totally order URI inside each partition. This is done through a lexical ordering of URI
  //   components (scheme, authority, path...), already implemented as `<` for `Uri`.
  //
  // Therefore, the URI comparison key is of type `int x int x int x Uri`, and allows for total
  // ordering through its lexical ordering.
  auto cmpKey(const Uri& u) -> UriCmpKey
  {
    using std::get;
    // z-index: true comes before false, so true goes to 0 and false goes to 1.
    auto z = [](bool b) -> int { return b ? 0 : 1; };
    auto k = ka::product(z(isRelativeEndpoint(u)), 0, 0, u);
    if (get<0>(k) == 0) {
      get<1>(k) = z(isSdRelativeEndpoint(u));
    }
    else if ((get<1>(k) = z(hasAuthority(u))) == 0) {
      get<2>(k) = z(hasLoopbackAddress(u));
    }
    return k;
  }
} // anonymous namespace
} // namespace detail

bool isPreferredEndpoint(const Uri& lhs, const Uri& rhs)
{
  using detail::cmpKey;
  return cmpKey(lhs) < cmpKey(rhs);
}

} // namespace qi
