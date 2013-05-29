**********************
QiMessaging JavaScript
**********************

.. warning::
   This library is still under development. Feel free to contact
   llec@aldebaran-robotics.com for more information.

Introduction
============

QiMessaging provides JavaScript bindings to call remote services through
a web browser using JSON.

`Socket.IO <http://socket.io/>`_ is used in order to establish full-duplex
communication between the robot and the browser, and easily deal with
connection state.

The library was designed around the `jQuery <http://www.jquery.com/>`_
`Deferred object <http://api.jquery.com/category/deferred-object/>`_, of which
most functions return a
`promise <http://api.jquery.com/deferred.promise/>`_.

The client page therefore requires three script inclusions: QiMessaging itself,
and its two dependencies: jQuery and Socket.IO.

.. code-block:: html

   <script src="../../libs/qimessaging/1.0/jquery.min.js"></script>
   <script src="../../libs/qimessaging/1.0/socket.io.min.js"></script>
   <script src="../../libs/qimessaging/1.0/qimessaging.js"></script>

.. note::
   The related path used above would correspond to a page hosted on
   ``http://nao.local/apps/airnao/index.html`` for instance. The absolute URL
   would be ``http://nao.local/libs/qimessaging/1.0/qimessaging.js``.

How to use
==========

The binding provides only one class: ``QiSession``, the JavaScript equivalent
of QiMessaging sessions. Once connected, it can be used to call services and
access the socket.

It is constructed using the URL of the JSON server.

.. code-block:: javascript

   var qim = new QiSession("http://" + window.location.host + ":8002");

.. note::
   Using ``window.location.host`` makes sure you connect to the same robot
   that is hosting the web page.

Once the connection is established, two methods are available: ``service()``
and ``socket()``.

QiSession.service()
===================

In case of success, this method calls the ``done()`` callback with an
object corresponding to the requested service.

.. code-block:: javascript

   function onService(service)
   {
   }

   qim.service("serviceTest").done(onService);

Services are JavaScript bound objects providing corresponding API :ref:`calls`
and :ref:`events`.

.. _calls:

Calls
-----

Service calls are only JavaScript function calls returning jQuery Deferred
promises, which means they are entirely asynchronous.

.. code-block:: javascript

   function onReply(data)
   {
     console.log(data);
   }

   function onError(data)
   {
     console.log(data);
   }

   function onService(service)
   {
     service.reply("plaf").done(onReply).fail(onError);
   }

   qim.service("serviceTest").done(onService).fail(onError);

   // console
   > plafbim

.. _events:

Events
------

Events are JavaScript objects inside a service, that provide two methods,
``connect()`` and ``disconnect()``. The first one will return an id that must
be used by the second one for unregistration.

.. code-block:: javascript

   function onMyEvent(data)
   {
     console.log('myEvent triggered, with:', data);
   }

   function onUnregister(data)
   {
     console.log('myEvent unregistered');
   }

   function onRegister(eventId)
   {
     service.myEvent.disconnect(eventId).done(onUnregister);
   }

   service.myEvent.connect(onMyEvent).done(onRegister);

QiSession.socket()
==================

This function will return the underlying `socket.io` object, that can
be used to deal with low-level
`socket events <https://github.com/LearnBoost/socket.io/wiki/Exposed-events>`_.

.. code-block:: javascript

   qim.socket().on('connect', function() {
     console.log('connected!');
     start();
   });

   qim.socket().on('disconnect', function() {
     console.log('disconnected!');
   });

Sample
======

.. code-block:: html

   <!DOCTYPE html>
   <html>

   <head>
   <title>QiSession example</title>
   <script src="../../libs/qimessaging/1.0/jquery.min.js"></script>
   <script src="../../libs/qimessaging/1.0/socket.io.min.js"></script>
   <script src="../../libs/qimessaging/1.0/qimessaging.js"></script>
   </head>

   <body>
   <script>
   var qim = new QiSession("http://" + window.location.host + ":8002");

   qim.socket().on('connect', function() {
     console.log('connected!');
     start();
   });

   qim.socket().on('disconnect', function() {
     console.log('disconnected!');
   });

   function onError(data)
   {
     console.log(data);
   }

   function onReply(data)
   {
     console.log(data);
   }

   function onService(service)
   {
     service.reply("plaf").done(onReply).fail(onError);
   }

   function start()
   {
     qim.service("serviceTest").done(onService).fail(onError);
   }
   </script>
   </body>

   </html>
