**********************
QiMessaging JavaScript
**********************

Introduction
============

QiMessaging provides JavaScript bindings to call remote services through
a web browser.

The server side uses the Python bindings, and listen for both HTTP and
Websocket requests.

The client side consists in a few JavaScript files, that can be embedded
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


Client side
===========

The client page will require three script inclusions: qimessaging itself,
and its two dependencies, `socket.io <http://socket.io/>`_ and
`jQuery <http://www.jquery.com/>`_.

.. code-block:: html

   <script src="http://qim.io/jquery.min.js"></script>
   <script src="http://qim.io/socket.io.js"></script>
   <script src="http://qim.io/qimessaging.js"></script>

The API was designed around the jQuery
`Deferred object <http://api.jquery.com/category/deferred-object/>`_, which
is returned by most functions.

The binding provides only one class: ``QiSession``, the JavaScript equivalent
of qimessaging sessions. It is constructed using the JSON server URL.

.. code-block:: javascript

   qim = new QiSession("http://nao.local:8080");

Once the connection is established, three methods are available ``services()``,
``service()`` and ``socket()``.


QiSession.services()
--------------------

This method will call the ``done()`` callback with a list of string,
corresponding to services registered to the *ServiceDirectory*.

.. code-block:: javascript

   function onServices(services)
   {
     console.log(services);
   }

   qim.services().done(onServices);


.. code-block:: javascript

   // console
   > ["ServiceDirectory", "serviceTest"]


QiSession.service()
-------------------

This method will call the ``done()`` callback with an object corresponding
to the requested service.

.. code-block:: javascript

   function gotService(onService)
   {
   }

   qim.service("serviceTest").done(onService);

Using services
^^^^^^^^^^^^^^

Once a service is retrieved through `QiSession.service()`, it is bound
to an object providing the corresponding APIs. Service call also return
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



QiSession.socket()
------------------

This last function will return the underlying `socket.io` object, that can
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
   <script src="http://qim.io/jquery.js"></script>
   <script src="http://qim.io/qimessaging.js"></script>
   <script src="http://qim.io/socket.io.js"></script>
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

   function onServices(services)
   {
     console.log(services);
     qim.service("serviceTest").done(onService);
   }

   qim.services().done(onServices);
   </script>
   </body>

   </html>
