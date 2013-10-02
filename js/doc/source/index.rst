**********************
QiMessaging JavaScript
**********************

Introduction
============

QiMessaging provides JavaScript bindings to call remote services (modules)
and subscribe to signals (events) in a web browser.

`Socket.IO <http://socket.io/>`_ is used in order to establish full-duplex
communication between the robot and the browser, and easily deal with
connection state.

The library was designed around the `jQuery <http://www.jquery.com/>`_
`Deferred object <http://api.jquery.com/category/deferred-object/>`_, of which
most function calls return a
`promise <http://api.jquery.com/deferred.promise/>`_.

Please make sure you are familiar with those libraries before starting.

How to use
==========

The client page requires three script inclusions: QiMessaging itself,
and its two dependencies: jQuery and Socket.IO. Assuming your page is
stored in the root folder of you application (``application/html/index.html``),
you should add the following lines:

.. code-block:: html

   <script src="../../libs/qimessaging/1.0/jquery.min.js"></script>
   <script src="../../libs/qimessaging/1.0/socket.io.min.js"></script>
   <script src="../../libs/qimessaging/1.0/qimessaging.js"></script>

.. note::
   The absolute URL would be
   ``http://nao.local/libs/qimessaging/1.0/qimessaging.js``.

.. warning::
   If you plan to use jQuery for your application, do not rely on the providing
   one, which can change. You may instead want to take a look at
   `jQuery.noConflict() <http://api.jquery.com/jQuery.noConflict>`_.

QiSession
=========

The bindings provide only one class: ``QiSession``, the JavaScript equivalent
of QiMessaging sessions, ie. a connection to the robot. Once connected, it can
be used to call services and access the socket.

It is constructed using the URL of the JSON server, and the Socket.IO
namespace.

.. code-block:: javascript

   var qim = new QiSession("http://" + window.location.host,
                           "libs/qimessaging/1.0/socket.io");

.. note::
   Using ``window.location.host`` makes sure you connect to the robot
   that is hosting the web page.

Once the connection is established, two methods are available: ``service()``
and ``socket()``.

QiSession.service()
===================

You can call this function to get a JavaScript proxy to any service, also known
as modules. Services are JavaScript bound objects providing corresponding API
:ref:`calls` and :ref:`signals`.

In case of success, this method calls the ``done()`` callback with an
object corresponding to the requested service. Otherwise, the ``fail()``
callback is called.

.. code-block:: javascript

   function onError(data)
   {
     console.log(data);
   }

   function onService(proxy)
   {
     proxy.say("Hello, world!");
   }

   qim.service("ALTextToSpeech").done(onService).fail(onError);

.. _calls:

Calls
-----

Service calls are only JavaScript function calls returning jQuery Deferred
promises, which means they are entirely asynchronous.

.. code-block:: javascript

   function onGetLanguage(data)
   {
     console.log(data);
   }

   function onError(data)
   {
     console.log(data);
   }

   function onTTSService(tts)
   {
     tts.say("hello").fail(onError);
     tts.getLanguage().done(onGetLanguage).fail(onError);
   }

   qim.service("ALTextToSpeech").done(onTTSService).fail(onError);

.. _signals:

Signals
-------

Signals are JavaScript objects inside a service, that provide two methods,
``connect()`` and ``disconnect()``. The first one will return an id that must
be used by the second one for unregistration.

.. code-block:: javascript

   eventId = 0;

   function onMySignal(data)
   {
     console.log('mySignal triggered, with:', data);
     service.mySignal.disconnect(signalId).done(onUnregister);
   }

   function onUnregister(data)
   {
     console.log('mySignal unregistered');
   }

   function onRegister(data)
   {
     eventId = data;
   }

   service.myEvent.connect(onMyEvent).done(onRegister).fail(onError);

.. note::
   ALMemory events cannot be directly used as QiMessaging signals. The sample
   below will help you do so.

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
   var qim = new QiSession("http://" + window.location.host,
                           "libs/qimessaging/1.0/socket.io");

   qim.socket().on('connect', function() {
     console.log('connected!');
     start();
   });

   qim.socket().on('disconnect', function() {
     console.log('disconnected!');
   });

   function onGetLanguage(data)
   {
     console.log("I speak " + data +  "!");
   }

   function onError(data)
   {
     console.log(data);
   }

   function onTTSService(tts)
   {
     tts.say("Hello Java Script world").fail(onError);
     tts.getLanguage().done(onGetLanguage).fail(onError);
   }

   function onTouch(val)
   {
     if (val == 1)
     {
       console.log("Purrrrrrrr");
     }
     else
     {
       console.log("Meow?")
     }
   }

   function onALSubscribed(signalId)
   {
     // signalId can be used to unregister
   }

   function onSubscribed(subscriber)
   {
     // The subscriber retrieved from ALMemory will help us get a signal
     subscriber.signal.connect(onTouch).done(onALSubscribed).fail(onError);
   }

   function onALMemoryService(alm)
   {
     // Extra step when dealing with ALMemory events
     alm.subscriber("FrontTactilTouched").done(onSubscribed).fail(onError);
   }

   function start()
   {
     qim.service("ALTextToSpeech").done(onTTSService).fail(onError);
     qim.service("ALMemory").done(onALMemoryService).fail(onError);
   }
   </script>
   </body>

   </html>

.. note::
   If you have any question or request, feel free to contact
   llec@aldebaran-robotics.com.
