.. _guide-py-service:

How to write a qimessaging service in Python
********************************************

Introduction
============

This guide will show you how to write a qimessaging service in Python, and advertise
it to others.


Prerequisites
-------------

- An installed python NAOqi SDK for your operating system.


Creating a service
------------------

We are going to create simple service that raises a signal when its method bang is called.

To do that, let's declare a simple class:

.. code-block:: python

  import qi

  class MyFooService:
    def __init__(self, *args, **kwargs):
      #define a signal 'onBang'
      self.onBang = qi.Signal()

    #define a bang method that will trigger the onBang signal
    def bang(self):
      #trigger the signal with 42 as value
      self.onBang(42)


Creating a session
------------------

To make the service available to others we need a :py:class:`qi.Session`.

Let's create a :py:class:`qi.Session` and connect it to a ServiceDirectory, then let's register our MyFooService created just above under the name 'foo'.

.. code-block:: python

  import qi

  #create an application
  app = qi.Application()

  #create an instance of MyFooService
  myfoo = MyFooService()

  s = qi.Session()
  s.connect("tcp://127.0.0.1:9559")
  #let's register our service with the name "foo"
  id = s.registerService("foo", myfoo)

  #let the application run
  app.run()


Test the service
----------------

We are going to create a simple client to call the service and react to it.

.. code-block:: python

  import qi

  def onBangCb(i):
    print "bang:", i

  s = qi.Session()
  s.connect("tcp://127.0.0.1:9559")
  foo = s.service("foo")
  #register a callback on 'onBang'
  foo.onBang.connect(onBangCb)
  #call bang
  foo.bang()

Wrapping Asynchronous API
-------------------------

If the API you are wrapping is asynchronous, you want to avoid blocking in your function just waiting for your asynchronous call to finish.

To avoid blocking your function you can use :py:class:`qi.Promise` and :py:class:`qi.Future`.

The procedure is the following:

- You create a :py:class:`qi.Promise`
- You create a handler that will set the value of the :py:class:`qi.Promise` when the asynchronous call finish
- You launch your asynchronous call giving it the handler, that will be called on completion
- You return the :py:class:`qi.Future` to your user

For the example we will implement a mycrazydelay and mycrazydelay_async.

.. code-block:: python

   # stupid implementation of a function that:
   #   - take time
   #   - call finish_callback
   #   - return the delay (or a the result of the computation)
   # if this function was stricly used synchronously the callback you be useless
   def mycrazydelay(delay, finish_callback):
       time.sleep(delay)
       if finish_callback:
          finish_callback(delay)
       return delay

   # simple wrapper around mycrazydelay to make it asynchronous
   def mycrazydelay_async(delay, finish_callback):
      qi.async(mycrazydelay, args=(delay, finish_callback))


and create a Foo service that call this two methods:

 - bar which wrap the synchronous mycrazydelay
 - betterBar which wrap the asynchronous mycrazydelay_async

.. code-block:: python

   class Foo:

       #block a thread for 50sec :(
       def bar(self):
           return mycrazydelay(50, None)


       def betterBar(self):
           p = qi.Promise()

           # callback that will set the value on the promise
           def finish(d):
              p.setValue(d)

           # return immediately
           mycrazydelay_async(50, finish)
           # return the future that will have the value set when p.setValue is called
           return p.future()

The two functions bar and betterBar behave exactly the same from a client side of view.
The first one will block a thread for 50sec, the second one is asynchronous, and keeps precious thread ressources free.
betterBar returns a Future, that will be notified when p.setValue is called.

.. note::

   returning a Future does not change what the client will see. If you return a Future containing a bool, the client will just see a bool. Future are not sent thought the network.

   So if you want to use the API asynchronously you have to use qi.async.

Let's look at a client example:

.. code-block:: python

   app = qi.Application()
   app.start()
   foo = app.session.service("foo")

   #both call are identical, they return a delay
   delay = foo.bar()
   delay = foo.betterBar()

   #to use them asynchronously:
   fut = qi.async(foo.bar)
   fut = qi.async(foo.betterBar)

   delay = fut.value()

Single-threaded / Multi-threaded mode for services
--------------------------------------------------

By default, python services are in single-threaded mode.
This means their methods cannot be called in parallel.
You might encounter this kind of message if you call several methods of such a module at the same time.

.. code-block:: console

  [W] 1394221129.982971 4079 qitype.dynamicobject: Time-out acquiring object lock when calling method. Deadlock?
  [E] 1394221129.988775 4052 python: RuntimeError: Time-out acquiring lock. Deadlock?
  [W] 1394221137.850762 3859 qitype.dynamicobject: Time-out acquiring object lock when calling method. Deadlock?

If you need to call methods of your module in parallel, you will need to protect them yourself,
and specify that your module is in multithread mode by using the qi.multiThreaded decorator.

.. code-block:: python

   @qi.multiThreaded()
   class Foo:
     #
     # Define your service
     #
