Architecture
============

Goals:
 - solid in the long term
 - unified architecture for naoqi and choregraphe
 - APIs that can be maitained for a long time

Communications
--------------

Multiples communications vectors:
Multiple way to connect to the robot:

 - secure : default local network connection. Choregraphe, etc...
   The secure connection is used to connect from a local network.

 - tcp : choregraphe, "realtime" apps
   disable by default. Could be enable to allow direct not secure tcp access to the robot. This should improve performance a little compared to a secure connection. This extra performance may be needed for very specific applications.

 - using a thirdparty : allow to communicate with a NAO from everywhere once both you and NAO are connected to the Internet.
   may be used by most remote application that could run everywhere. For example application on phones, tablets, laptops..

It would be great to have a unified way to connect to the robot, that would allow to connect all our sotware using (fast) secure connection when local and xmpp otherwize.

ROS/YARP/..: other robotics framework can allow to connect to NAO too. If bridge between NAOqi and thouse frameworks are added to the NAOStore they will be very easy to use.

Applications
------------

 - embedded application: Application loaded on the robot. They are made of services and behaviors.


 - remote application: application that interact with the robot. Could be using a thirdparty server to connect to the robot, or using a direct connection to the robot.
   They can be architectured like the client want, because they do not need to be integrated/launched by NAO's life.
   This include scripts.


Basic patterns
--------------

Behavior:
Most behaviors are asynchronous, basically a behavior is sequence of action. It's a graph of connected objects. Object can receive and send events. They react on event by doing computation and sending other events. It's the concept behind choregraphe boxes. It's also how one would like to write behaviors in others languages. Could be integrated in the NAO's life.

Service:
Service expose functionnality to behaviors and outside applications. They respond to request and publish data. They may not be multithreaded if they only have one task to do. A task queue could be added to enqueue orders.

Scripts:
Should be easy to write. It's mostly for testing purpose. Motion want to test the new walk, someone want to test a specific API.

We should concentrate on writing behaviors instead of scripts, it's what we will have the most, and it's integrated with NAO's life.

It should be easy to write service too.



What is needed?

Service:
--------
They expose our APIs to the outside and to behaviors.

interacting with others:
  - raise global events
  - receive and handle orders/commands
  - receive and send streams
  - be configurable (have a set of properties)
  - have settings (accessible from the PreferenceManager)

APIs:

apis for a service should provide a way to
  - call methods
  - connect streams to signals/slots
  - call remote actions
  - instanciate local object corresponding to command. (local or not)


Behavior:
---------
should be easy to use behavior from choregraphe, python, c++, etc.. togethers.
If we want behaviors to be used inside others behaviors, we need them to have a single format. It should be binary if we want to support compiled languages. It should be easy to group them into a single place.
It seems logical to choose library as the standard format.
If it's a python module then the library will load a python interpreter and load the needed code. A library by external language should be enough. For python we could always reuse the same library for example. This library will enumerate all objects and allow to instanciate them. For behaviors a default object should be set. We could optimize a little for python and choregraphe behaviors by providing the support library by default.

interacting with others:
  - slot: receive an event (typed or not)
  - signal: send an event (typed or not)
  - properties: (configurations)

store, life, etc..
  - metadata for NAO's life
  - xar that describe objects and connections between them


Xar can be edited using Choregraphe. Choregraphe basically loads boxes and connect them using the FrameManager.

Some standard "boxes"/"commands"/"actions" are provided by the system. For example: say, walk, move, look. When used in choregraphe or more generally in behavior they are represented by a remote action. "a remote command".



