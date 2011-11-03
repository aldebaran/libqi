Choregraphe
===========

Choregraphe is provided with all XAR for each targeted versions.

If choregraphe can run on 1.12 and 1.14 then he have the 1.12 and 1.14 set of XAR.

Choregraphe connect to a robot that have a specific version (1.12 or 1.14) and know all available services.
All call in choregraphe are asynchronous.

Behavior/Object/TimeLine/Filter.

We need to be able to write behavior in multiple languages. At least we want all program to be usable like a behavior/box.
=> A behavior is a library that we can load.
=> behavior can be used like box. (is that the link box?)

Compatibility
-------------

We may support backward compatibility through "network protocol versionning". The critical part that always need to be backward compatible is the network protocol.

We may imagine a way to understand message from multiple API versions. Like protobuf do.
