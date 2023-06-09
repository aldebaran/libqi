libqi Change Log
=================

libqi 4.0.1
-----------

This release focuses on fixing some bugs, rewriting and improving the
project build definition, and porting library code to OpenSSL 3 and
Boost 1.78.

### Build

The definition of the build of libqi as be rewritten to remove the
dependency to qibuild and qi toolchains and instead use more standard
tools and conventions.

The CMakeLists.txt files have been completely refactored to use an approach
closer to what is referred to as "modern CMake". The project now requires
CMake 3.23 or greater.

A Conan 2 project definition file has also been added so that Conan 2 may
be used to fetch the dependencies of the project or to create a package.

### OpenSSL 3

The library used to depend on OpenSSL 1, which will soon no longer be
maintained. It now depends on OpenSSL 3.

### Boost 1.78

The library now supports Boost 1.78.

### Fixes

- Member functions that are declared `noexcept` are now advertised
  properly. #SW-2913
- Build errors have been fixed when compiling with Clang 12 and more.
- Some compiler warnings have been fixed.

libqi 4.0.0
-----------

This release sets the version of C++ supported by libqi to C++17. Prior
versions of C++ are not supported anymore.

### API Changes

- If the `[[fallthrough]]` attribute is available (introduced in C++17),
`QI_FALLTHROUGH` expands to it. Before the attribute was available, it
was, in most cases, expanded to ((void)0).

- `qi::AnyValue` can now contain `std::optional` values. More generally,
the `std::optional` types are now supported in the type system of
`qi`.

- The conversion operator of `qi::Future<T>` and `qi::FutureSync<T>` to
`const T&` has been removed. It has been deprecated for several years and
is considered harmful. In order to access the result of a future, user code
must now either create a continuation with `then` or `andThen` or
explicitly block until the result is available with `value` or `wait`.

libqi 3.0.1
-----------

This version fixes a number of internal memory errors, including bad accesses
and leaks. It also corrects an error concerning Unicode characters and locales
in JSON serialization.

libqi 3.0.0
-----------

This release aims to add the support of mTLS with peer certification
verification to the library.

### API changes

#### New header: 'qi/messaging/tcpscheme.hpp"

This header introduces a new enumeration `qi::TcpScheme`.

This enumeration has been added in an effort to remove usage of URL "protocol"
values as raw strings. It contains three values, each associated to a URI scheme:
  - `TcpScheme::Raw` associated to the 'tcp' scheme.
  - `TcpScheme::Tls` associated to the 'tcps' scheme.
  - `TcpScheme::MutualTls` associated to the new 'tcpsm' scheme.

With this new type come a few utility functions, such as a conversion function
from URL/URI or strings into `TcpScheme` values.

#### New header: 'qi/messaging/ssl/ssl.hpp'

This header introduces a new namespace `qi::ssl` which contains types and
functions around the OpenSSL library.

The header introduces the following types (among less significant others):
  - `qi::ssl::BIO`: a basic input/output wrapper.
  - `qi::ssl::Certificate`: a X509 Certificate.
  - `qi::ssl::PKey`: a private key.
  - `qi::ssl::CertChainWithPrivateKey`: a pair of a X509 certificates chain and
    a private key.
  - `qi::ssl::ClientConfig`: the configuration of a client doing SSL over TCP.
  - `qi::ssl::ServerConfig`: the configuration of a server doing SSL over TCP.
  - `qi::ssl::Error`: an error type usually thrown when an SSL error occurs in
    our code.
  - `qi::ssl::PemPasswordCallback`: a callback to decrypt a PEM object
    protected by a password.

#### `qi::Session`

A few fields were added to the `qi::SessionConfig` type:
  - `clientSslConfig`: the client side SSL configuration for the session.
  - `serverSslConfig`: the server side SSL configuration for the session.
  - `clientAuthenticatorFactory`: added for consistency, the factory used by
    the client side of the session for qi based authentication.
  - `authProviderFactory`: added for consistency, the factory used by the
    server side of the session for qi based authentication.

However, some of these fields are not comparable. Therefore,
`qi::SessionConfig` is also not comparable anymore.

The `qi::Session::endpoints()` member function returns a list of endpoints
now ordered by preference according to the `qi::isPreferredEndpoint`
predicate.

The constructor `qi::Session(bool enforceAuth, SessionConfig)` is now
deprecated. Enforcing authentication should now be done by setting the
authentication provider factory and the client authenticator factory of the
session configuration structure.

The `qi::Session::setIdentity(...)` member function has been removed. The
certificate and the key of either side of the session should now be set in the
session configuration structure.

#### `qi::ApplicationSession`

As a result of `qi::SessionConfig` not being comparable,
`qi::ApplicationSession::Config` is also not comparable anymore.

#### `qi::Gateway`

The previous `qi::Gateway` structure has been replaced by a type alias to
`qi::ServiceDirectoryProxy`.

A `qi::GatewayPtr` type alias to `qi::ServiceDirectoryProxyPtr` has been added.

#### `qi::ServiceDirectoryProxy`

The `qi::ServiceDirectoryProxy` has been reworked in an effort to reduce its
code complexity.

All the parameters of the proxy (including the service directory URL, the list
of listen URL, the service filter, the SSL parameters and the authentication
factories) are now set through a configuration structure
`qi::ServiceDirectoryProxy::Config` that is passed at construction of the proxy.

Previous member functions that would change the state of the proxy have been
removed, including:
  - `qi::ServiceDirectoryProxy::attachToServiceDirectory(...)`.
  - `qi::ServiceDirectoryProxy::listenAsync(...)`.
  - `qi::ServiceDirectoryProxy::setServiceFilter(...)`.
  - `qi::ServiceDirectoryProxy::setValidateIdentity(...)`.

The class now offers a static member function
`qi::ServiceDirectoryProxy::create(Config)` returning a
`qi::Future<qi::ServiceDirectoryProxyPtr>` that is set once the proxy is fully
operational, i.e. it is connected to the service directory and its server is
running. This ensures that service directory proxy objects are less stateful
and simplifies user code.

Since a proxy can still lose connection from its service directory, the
`status` property is still available. The `ListenStatus` and `ConnectionStatus`
enumeration have been merged into one `Status` enumeration. Its values are
limited to the following values:
  - `Status::Initializing`: the proxy is still initializing and has not
    launched a connection to the service directory yet.
  - `Status::Created`: equivalent to `Status::Initializing`.
  - `Status::Connecting`: the proxy is trying to connect to the service directory.
  - `Status::Initialized`: equivalent to `Status::Connecting`.
  - `Status::TryingToListen`: the proxy is connected to the service directory and is
    trying to launch its server.
  - `Status::Connected`: equivalent to `Status::TryingToListen`.
  - `Status::Listening`: the proxy is connected and its server is operational.
  - `Status::Ready`: equivalent to `Status::Listening`.

To avoid exposing unnecessary implementation details, a static
`qi::ServiceDirectoryProxy::Config::createFromListeningProtocol` method has
been added.

`qi::ServiceDirectoryProxy::Filepaths` is a sum type corresponding to the file
paths needed to create a configuration for each listening protocol.

Since there is no logic based on `Gateway::Config` itself,
`qi::ServiceDirectoryProxy::createFromListeningProtocol` is provided that
passes arguments as-is to
`qi::ServiceDirectoryProxyPtr::Config::createFromListeningProtocol`.


libqi 2.1.0
-----------

This release mainly aims to upgrade some used libraries and the C++ dialect, as well as some tools.

### Upgrades

- Boost: from 1.64 to 1.77
- C++: the required version of the standard has been changed from C++11 to C++14.
- CMake: minimum version from 2.8 to 3.16 (the version coming with Ubuntu 20.04)

### API changes

- `ka.typetraits`: Renames `HasInputIteratorTag` into `IsInputIterator`
  (API break, but only used internally).
- `ka.typetraits`: Adds type predicates checking the category of an iterator type
  (input, forward, bidirectional, random access).


libqi 2.0.0
-----------

This release provides a few fixes.
Particularly, on OS using `systemd`, when compiling with the `WITH_SYSTEMD` flag, it fixes `localhost`
communication issues due to the qi-messaging's machine-id implementation.
Compatibility breakage on OS using `systemd`: The behavior change when `WITH_SYSTEMD` is defined at
compile time, makes it incompatible with older versions of libqi running on the same machine, when
`localhost` is the only available endpoint.
This change shall be considered a qi-messaging protocol breakage.


libqi 1.8.7
-----------

This release mainly ports the tests on `GoogleTest 1.10.0`, and improves the compatibility with
more recent versions of `boost`.


libqi 1.8.6
-----------

This release aims to improve security and reliability.


libqi 1.8.3
-----------

This release aims to improve TLS connection security (#45843) and to fix a connection backward
compatibility issue (#45842).


libqi 1.8.2
-----------

This release focuses on fixing a blocking issue on the service directory proxy (mirroring) (#45483).


libqi 1.8.1
-----------

This release focuses on enabling remote access to service gateways on a machine hidden behind another,
by adding NAT support.


libqi 1.8.0
-----------

This release focuses on improving security.
Particularly, libqi now requires `TLS 1.2` on server side.


libqi 1.7.2
-----------

This release focuses on improving reliability of libqi client side.


libqi 1.7.0
-----------

This release focuses on improving reliability and memory consumption.
It includes fixes to libqi.

Notice that this release introduces a small API breakage by changing `qi::Signal<T>` and `qi::Property<T>`
to be no more a `boost::function<T>`.
However `qi::SignalF<T>` still can be converted to `boost::function<T>` implicitely.


libqi 1.6.15
------------

This release focuses on improving security and fixes some gateway crashes.


libqi 1.6.14
------------

This release fixes a few qi-messaging issues.
