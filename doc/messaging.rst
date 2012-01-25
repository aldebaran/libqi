Messaging
=========

Networking
----------

We support multiple communications way. We have need a way to select which communication means we should use.

.. image:: /medias/qimessaging-networking.png

- lpc
- ipc
- local networking: tcp/ip (romeo)
- local networking: gateway tcp/ip
- internet xmpp:

A service can have multiple endpoints represent a way to connect the service, for example ipc and tcp.
To connect client to their final service we need a ServiceDirectory which enumerate services and endpoints associated to them.

Use Case
--------
Romeo: multiple ATOM boards.

ServiceDirectory
----------------

Only service on lpc/ipc/rpc can be registered. It should not be possible to register service behind a gateway nor XMPP.
Clients connected using the gateway will continue using the same gateway whatever happend.
The endpoint selection mecanism apply only to client on on at least the same subnetwork.



LPC:
^^^^
When: same machineId and processId.
pointer on the service or proxy object with pointers.

IPC:
^^^^
When: same machineId

.. image:: /medias/network_without_gateway.png

RPC:
^^^^
When: different machineId

Same diagram than IPC.

GateWay:
^^^^^^^^
When: different machineId

.. image:: /medias/network_with_gateway.png


XMPP:
^^^^^
When:


How client work:
// ip/port of the master
qi::Client(ip, port)

* connect to the gw/master
* getServiceEndpoint
    * lpc/ipc are obvious
    * same subnet: rpc
    * client connected though gw (using external address): continue using the gw


Basic brick
-----------

MethodCall
^^^^^^^^^^
one input (onStart), two output (onStopped, onError), no parameters
executed by a thread pool. method should not take too long to process or they are killed.
Method cant be stopped. Synchronous by essence. (could be used asynchronous by choregraphe (represented by a box))

Streams
^^^^^^^
one input, or one output.
multipublisher/multisubscriber?

Message (shm)
^^^^^^^^^^^^^
Message can be allocated in shm to avoid copy. This is very important for audio/video stream.

FD passing
^^^^^^^^^^
we can pass fd between process to allow then to share memory block and files?

ThreadQueue
^^^^^^^^^^^

Internal Machinery
------------------

A TransportClient can connect to a remote Service. he create two streams (one in each way).
This is how he communicate with a service.
Message are send through one pipe and receive though the other. Messages are typed "methodcall", "stream"

.. image:: /medias/network_machinery.png



Complex brick
-------------

Command
^^^^^^^
command are represented by a thread on the server, command receive input and emit output.
One running command = One thread.

Events
^^^^^^

event are broadcasted to all subscribers. Events are global.



Supported type
--------------

custom structure.




Python
------

support lambda?

connect(a, "finished", None, lambda x: print "yeah")




Type System
-----------

All type must be registered.
We should remove the maximum of

MetaProperty
MetaMethod
MetaClass
MetaData

a qi::Object always have a metadata describing all it's properties and methods (signal/slot).

A class can be constructed using MetaClass provided it has a default ctor. Then it can be populated using properties.


