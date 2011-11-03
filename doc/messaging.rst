Messaging
=========

Basic brick
-----------

MethodCall
++++++++++
one input (onStart), two output (onStopped, onError), no parameters
executed by a thread pool. method should not take too long to process or they are killed.
Method cant be stopped. Synchronous by essence. (could be used asynchronous by choregraphe (represented by a box))

Streams
+++++++
one input, or one output.
multipublisher/multisubscriber?

Message (shm)
+++++++++++++
Message can be allocated in shm to avoid copy. This is very important for audio/video stream.

FD passing
++++++++++
we can pass fd between process to allow then to share memory block and files?

ThreadQueue
+++++++++++

Internal Machinery
------------------

A TransportClient can connect to a remote Service. he create two streams (one in each way).
This is how he communicate with a service.
Message are send through one pipe and receive though the other. Messages are typed "methodcall", "stream"



Complex brick
-------------

Command
+++++++
command are represented by a thread on the server, command receive input and emit output.
One running command = One thread.

Events
++++++

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
