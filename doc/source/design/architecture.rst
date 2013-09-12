.. _architecture:

NAOqi 2.0 - Architecture
========================

Goals:
 - application oriented
 - be connected (to smartphone, tablet, computer, ... from everywhere)
 - be generic : basic building block could be used everywhere
 - be reusable : support NAO, Romeo, whatever...
 - be smart when applicable
 - be secure, solid, stable
 - boxes/c++ API unification
 - maintainable APIs
 - easy to understand and to debug

Application oriented
--------------------

The goal of the NAOqi is to ease the development of application. Application use NAOqi to communicate together.
We should port a particular attention at making writing and distributing application as simple as possible.
Application can be written for multiples devices, we need them to be self-contained and written in multiple languages.
Application should be easy to distribute and may contain multiples languages. Application should work across multiples
version of the SDK.

see :ref:`application`.

use case:

 - Distribute application for a robot using a Store
 - Distribute a program for PC (mac/linux/windows) that connect to a Robot
 - Distribute application for phones and tablets

Connected
---------

We need to overhaul the networking stack, we have multiple needs:

 - local: we need fast communication between applications on the robot
 - remote: it should be easy to connect tools and programs to a robot
 - cloud: we want to connect multiple devices together in a secure way

To that matter we need fast internal communication method, and a simple way to create secure gateway between the robot and the outside.

Gateway will allow to set specific permissions, will support a secure connection (for tools running on computers), will support connection
to a third party server (to support tablet and access from internet) and allow implementing support for others robotics frameworks.

use case:

 - Researcher want very fast networking to transfer audio/video/events and process them on a deported computer.
 - Researcher want to use differents robotics frameworks.
 - Application on the robot (maybe on multiple board) need fast communication between them.
 - Device need to be connected to each others and exchange informations.

:ref:`qimessaging-index`

Generic
-------

We dont know what the future is made of, so we need generic building blocks that we will be able to reuse to construct more advanced blocks.
The core of the framework should be generic enough to support all robotics and embedded devies use case.

Reusable
--------

The framework should provide API that are generic enough to be adapted to differents types of robots, we have no good reason to taillor
our interface to a specific robots, thinking generic and reusable will improve the quality of the whole software stack.

Smart
-----

Secure/Solid/Stable
--------------------

The framework should never be an issue. If failure occur, that should always restart and return in a good running state.


Security is an integral part of our platform, we dont want robot to become evil,
Remote Application must use a sessionId to authenticate on the robot. This sessionId should be configured on the robot, could be revoked at any time.

see :ref:`security`


Tracing / debuggable
--------------------

We need a tracing infrastructure to understand what's going on in the system.
We need to be able to display current running task, bandwidth usage for each services, frequency of calls and events.

It should be possible to activate the debug mode without too much performance penality.

We need a graphical tool to follow what's going on.
Choregraphe can display objects it seems pretty adapted to display activity between services/behaviors.
We could support an inspect mode, where we can see what's going on the wire, what current task are being run, what function take time (time of execution of a slot).
