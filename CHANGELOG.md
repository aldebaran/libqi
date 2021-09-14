libqi Change Log
=================


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