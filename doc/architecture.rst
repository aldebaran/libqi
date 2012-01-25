.. _architecture:

NAOqi 2.0 - Architecture
========================

Goals:
 - be connected (to smartphone, tablet, computer, ... from everywhere)
 - be generic : basic bulding block could be used everywhere
 - be reusable : support NAO, Romeo, whatever...
 - be smart when applicable
 - be secure, solid, stable
 - boxes/c++ API unification
 - maintainable APIs

Communications
--------------

Multiple way to connect to the robot:

 - secure : default local network connection. Choregraphe, etc...
   The secure connection is used to connect from a local network.

 - tcp : choregraphe, "realtime" apps
   disable by default. Could be enable to allow direct not secure tcp access to the robot. This should improve performance a little compared to a secure connection. This extra performance may be needed for very specific applications.

 - using a thirdparty server: allow to communicate with a NAO from everywhere once both you and NAO are connected to the Internet.
   may be used by most remote application that could run everywhere. For example application on phones, tablets, laptops..

It would be great to have a unified way to connect to the robot, that would allow to connect all our sotware using (fast) secure connection when local and xmpp otherwize.

ROS/YARP/..: other robotics framework can allow to connect to NAO too. If bridge between NAOqi and thouse frameworks are added to the NAOStore they will be very easy to use.

:ref:`qimessaging-index`

Applications
------------

The whole point of the NAOqi framework is to write applications. Application can run on the robot or be remote. We support both workflow.

see :ref:`application`.

Security
--------

Security is an integral part of our platform, we dont want robot to become evil,
Remote Application must use a sessionId to authenticate on the robot. This sessionId should be configured on the robot, could be revoked at any time.

see :ref:`security`


Tracing
-------

We need a tracing infrastructure to understand what's going on in the system.

We need to be able to display current running task, bandwith used for each services, frequency of calls for each methods.

It should be possible to activate the debug mode without too much performance penality.

We need a gui to follow what's going on. Choregraphe can display objects it seems pretty adapted to display activity between services/behaviors. We could support an inspect mode, where we can see what's going on the wire, what current task are being run, what function take time (time of execution of a slot).

What is needed?
