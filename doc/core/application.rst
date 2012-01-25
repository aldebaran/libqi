.. _application:

Application
===========

An application is a program that can run on the robot, or on other devices. An application can be distributed on the NAOStore.

We can distinguish two types of applications:

 - embedded application: Application loaded on the robot. They are made of
   services and behaviors.

 - remote application: application that interact with the robot. Could be using
   a thirdparty server to connect to the robot, or using a direct connection to
   the robot. They can be architectured like the client want, because they do
   not need to be integrated/launched by NAO's life. This include scripts.

We want application to be selfcontained to be easily distributable on the NAOStore. They should embbeded all their dependencies or have an easy way to tel user how to get them. (download another apps on the NAOStore?)


Applications components
-----------------------

Behavior:
Most behaviors are asynchronous, basically a behavior is sequence of action. It's a graph of connected objects. Object can receive and send events. They react on event by doing computation and sending other events. It's the concept behind choregraphe boxes. It's also how one would like to write behaviors in others languages. Could be integrated in the NAO's life.

Service:
Service expose functionnality to behaviors and outside applications. They respond to request and publish data. They may not be multithreaded if they only have one task to do. A task queue could be added to enqueue orders.

Scripts:
Should be easy to write. It's mostly for testing purpose. Motion want to test the new walk, someone want to test a specific API.

We should concentrate on writing behaviors instead of scripts, it's what we will have the most, and it's integrated with NAO's life.

It should be easy to write service too.



Embedded Application
--------------------

Embedded application are made of services and behaviors. An application that crash should not bring the whole systeme down, only the application should be impacted. To do that we need to isolate each application, see :ref:`local-security` for more details. Application could pack services and behaviors in the same process or use differents processes. Embedded application should provide a manifest to be integrated with other services (NAO's life, NAOStore, ..).


Remote Application
------------------

Remote application design is free, they connect to the robot and interact with him. There should be a mecanism to support remote application on the robot, see :ref:`life-remote-app` for more details.

.. _service:

Service
-------

What is this?
+++++++++++++

They expose our APIs to the outside and to behaviors.

What they need
++++++++++++++

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


.. _behavior:

Behavior
--------

What is this?
+++++++++++++

should be easy to use behavior from choregraphe, python, c++, etc.. togethers.
If we want behaviors to be used inside others behaviors, we need them to have a single format. It should be binary if we want to support compiled languages. It should be easy to group them into a single place.
It seems logical to choose library as the standard format.
If it's a python module then the library will load a python interpreter and load the needed code. A library by external language should be enough. For python we could always reuse the same library for example. This library will enumerate all objects and allow to instanciate them. For behaviors a default object should be set. We could optimize a little for python and choregraphe behaviors by providing the support library by default.


What they need?
+++++++++++++++

interacting with others:
  - slot: receive an event (typed or not)
  - signal: send an event (typed or not)
  - properties: (configurations)

store, life, etc..
  - metadata for NAO's life
  - xar that describe objects and connections between them


Xar can be edited using Choregraphe. Choregraphe basically loads boxes and connect them using the FrameManager.

Some standard "boxes"/"commands"/"actions" are provided by the system. For example: say, walk, move, look. When used in choregraphe or more generally in behavior they are represented by a remote action. "a remote command".



Application entrypoint
----------------------

An application could be a single process or multiple processes.

How to write a service in other languages
+++++++++++++++++++++++++++++++++++++++++

GNIIII????

command line handling:
  --start "service-or-behavior" --start "blabla"
  --resume "resumeID"


