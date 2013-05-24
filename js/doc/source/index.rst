**********************
QiMessaging JavaScript
**********************

.. warning::
   This library is still under development. Feel free to contact
   llec@aldebaran-robotics.com for more information.

Introduction
============

QiMessaging provides JavaScript bindings to call remote services through
a web browser.

It currently consists of two pieces of software: the server side, which
will listen for requests from the browser, and the client side, which is
the JavaScript library *per se*.

The server side uses the Python bindings, and listen for both HTTP and
Websocket requests.

The client side consists in a few JavaScript files that can be embedded
within a web page. The implementation relies on jQuery and socket.io.

Server side
===========

The JavaScript bindings need a running Websocket server with a qimessaging
session opened.

A script is provided, ``qim.py``, which uses
`Tornadio <https://github.com/facebook/tornado>`_ and
`Tornadio2 <home/laurent/src/master/lib/qimessaging/js/doc/source>`_ as
web backends.

It should be run as following:

.. code-block:: sh

   $ python2 qim.py tcp://nao.local:9559

The script will open two ports: ``8001`` for standard HTTP requests and
``8002`` for websocket connection. All mention to ``localhost`` in this page
refer to the machine where this script is running.

Client side
===========

The client page will require three script inclusions: qimessaging itself,
and its two dependencies, `socket.io <http://socket.io/>`_ and
`jQuery <http://www.jquery.com/>`_.

.. code-block:: html

   <script src="http://localhost:8001/jquery.min.js"></script>
   <script src="http://localhost:8001/socket.io.min.js"></script>
   <script src="http://localhost:8001/qimessaging.js"></script>

The API was designed around the jQuery
`Deferred object <http://api.jquery.com/category/deferred-object/>`_, of which
most functions return a
`promise <http://api.jquery.com/deferred.promise/>`_.

The binding provides only one class: ``QiSession``, the JavaScript equivalent
of qimessaging sessions. It is constructed using the JSON server URL.

.. code-block:: javascript

   qim = new QiSession("http://localhost:8001");

Once the connection is established, two methods are available: ``service()``
and ``socket()``.

QiSession.service()
-------------------

In case of success, this method will call the ``done()`` callback with an
object corresponding to the requested service.

.. code-block:: javascript

   function onService(service)
   {
   }

   qim.service("serviceTest").done(onService);

Calls
^^^^^

Once a service is retrieved through `QiSession.service()`, it is bound
to an object providing the corresponding APIs. All service calls return
Deferred objects.

.. code-block:: javascript

   function onReply(data)
   {
     console.log(data);
   }

   function onService(service)
   {
     service.reply("plaf").done(onReply);
   }

   qim.service("serviceTest").done(onService);

   // console
   > plafbim

Events
^^^^^^

Events are also available. These are JavaScript objects providing two methods,
``connect()`` and ``disconnect()``. The first one will return an id that must
will be used by the second for unregistration.

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
------------------

This function will return the underlying `socket.io` object, that can
be used to deal with low-level
`events <https://github.com/LearnBoost/socket.io/wiki/Exposed-events>`_.

.. code-block:: javascript

   qim.socket().on('disconnect', function() {
     console.log('disconnected!');
   });

Complete example
----------------

.. code-block:: html

   <!DOCTYPE html>
   <html>

   <head>
   <title>QiSession example</title>
   <script src="http://localhost:8001/jquery.min.js"></script>
   <script src="http://localhost:8001/socket.io.min.js"></script>
   <script src="http://localhost:8001/qimessaging.js"></script>
   </head>

   <body>
   <script>
   qim = new QiSession("http://nao.local:8080");

   qim.socket().on('disconnect', function() {
     console.log('disconnected!');
   });

   function onReply(data)
   {
     console.log(data);
   }

   function onService(service)
   {
     service.reply("plaf").done(onReply);
   }

   qim.service("serviceTest").done(onService);
   </script>
   </body>

   </html>
