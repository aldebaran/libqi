qi::Future
==========
.. _api-future:

.. code-block:: c++

  #include <qi/future.hpp>

.. cpp:namespace:: qi

A Future provides a way to wait for and get the result of an asynchronous
operation. It is the receiving end of a :cpp:class:`qi::Future` - :cpp:class:`qi::Promise` pair.


The different states of a Future
--------------------------------

A future can be in multiple states represented by :cpp:class:`FutureState` :

- FutureState_None: The Future is not tied to a Promise, and will never change state.
- FutureState_Running: The future is tied to a Promise, and the asynchronous
  operation has not finished yet.
- FutureState_Canceled: The operation was successfully canceled.
- FutureState_FinishedWithError: The operation finished with an error.
- FutureState_FinishedWithValue: The operation finished and its return value is available.

Getting the state and waiting for a Future
------------------------------------------

There are multiple ways to handle a future. The first is to *ignore* it,
and cast it to the underlying value type, or use the :cpp:func:`qi::Future<T>::value` method.
In that case, the call will block until the Future leaves the *running* state.
Then if a value if available, it will be returned. Otherwise a :cpp:class:`FutureException`
will be raised:

.. code-block:: cpp

  qi::Future<int> someOperation();

  int callIt()
  {
    int i = someOperation(); // wait, then get the value or throw
    return i;
  }

If you do not wish to wait forever, or want to handle Future error without
catching an exception, you can use :cpp:func:`qi::Future<T>::wait` (timeout):
This function waits at most the specified time in milliseconds, and return
a :cpp:enum:`FutureState`. You can then safely call :cpp:func:`qi::Future<T>::value` or
:cpp:func:`qi::Future<T>::error`, if future is in state FutureState_FinishedWithValue or
FutureState_FinishedWithError respectively:

.. code-block:: cpp

  qi::Future<int> f = someOperation();
  switch(f.wait(1000))
  {
     case FutureState_Running:
       std::cerr << "Still not ready" << std::endl;
       break;
     case FutureState_Canceled:
       std::cerr << "Canceled" << std::endl;
       break;
     case FutureState_FinishedWithError:
       std::cerr << "Error: " << f.error() << std::endl;
       break;
     case FutureState_FinishedWithValue:
       std::cerr << "Value: " << f.value() << std::endl;
       break;
  }

Future notification
--------------------

Alternatively, you can get notified of Future completion asynchronously using
:cpp:func:`qi::Future<T>::connect`. This function accepts a callback function or
functor with signature *void (qi::Future<T> f)*.

The Future guarantees you that your callback function will be called once and
only once, when or if the Future leaves the *Running* state (that is, enters
one of *Canceled*, *FinishedWithError* or *FinishedWithValue*):

.. code-block:: cpp

  void myCallback(qi::Future<int> f)
  {
    qi::FutureState s = f.wait(); // will return immediately, Future has finished.
    switch(s) {...}
  }

  <...>
   qi::Future<int> f = someOperation();
   f.connect(&myCallback);


The thread in which the callback invocation is made is up to the Promise,
so you should not make any assumption about it, or about whether the callback
is invoked synchronously to the Future end.

connect() accepts extra arguments after the callback: values or placeholders
that will be bound to the call(similarly to how *boost::bind* works). If
the first argument is a boost::weak_ptr, or inherits from :cpp:class:`qi::Trackable`,
then the callback will not be called if the weak_ptr cannot be locked, or
if the Trackable was destroyed:

.. code-block:: cpp

  class Foo
  {
    public:
    void onOpFinished(qi::Future<int> op, int opNumber);
  };
  void safe_async_op(boost::shared_ptr<Foo> foo, int opNumber)
  {
    qi::Future<int> future = someOperation();
    // This version will keep foo alive at least until the Future finished
    future.connect(&Foo::onOpFinished, f, _1, opNumber);
    // This version is safe in case foo is destroyed before the Future finishes.
    future.connect(&Foo::onOpFinished, boost::weak_ptr<Foo>(f), _1, opNumber);
  }

Future cancellation
-------------------

An async operations that returns a Future can support cancellation.
To check if a future you have can be canceled, use :cpp:func:`qi::future<T>::isCancelable`.
If if returns true, you can try to abort the operation by calling
:cpp:func:`qi::future<T>::cancel`. Depending on the operation and on
the timing of your call, your cancel request might be ignored (for example,
if it is received too late and a value is already available). But you can
expect the Future to hastily leave the *Running* state one way or an other.


qi::Promise
===========
.. _api-promise:

A :cpp:class:`qi::Promise` is an object that can create and satisfy :cpp:class:`qi::Future`.
Like *Future*, it has shared semantics (all copies of a Promise represent the
same object). The next example illustrates it's basic use case:

.. code-block:: cpp

  qi::Future<int> myFunctionReturningAFuture()
  {
    qi::Promise<int> promise;
    // start an asynchronous operation, holding the promise
    boost::thread(someAsynchronousOp, promise);
    return promise.future();
  }

  void someAsynchronousOp(qi::Promise<int> promise)
  {
     try {
       int result = performSomeTask();
       promise.setValue(result);
     }
     catch(const std::exception& e)
     {
       promise.setError(e.what());
     }
  }

In plain English:

- Create a *Promise* and return the future() obtained with :cpp:func:`qi::Promise<T>::future`.
- Transmit the *Promise* to the asynchronously executing code.
- Notify of successful completion with :cpp:func:`qi::Promise<T>::setValue` or :cpp:func:`qi::Promise<T>::setError`.
- Only one of the two functions above must be called, and only once per *Promise*.

Supporting cancellation
-----------------------

If your asynchronous operation can be canceled, you must provide a callback
with signature *void(qi::Promise<T>)* to the *Promise* constructor.

This callback will then be called if a cancellation request is received by a
connected *Future*. This callback is expected to ensure that the connected *Future*
hastily leave the *Running* state, by calling one of :cpp:func:`qi::Promise<T>::setValue`, :cpp:func:`qi::Promise<T>::setError` and :cpp:func:`qi::Promise<T>::setCanceled`.
However this call does not have to be made synchronously.


Controlling callback execution
------------------------------

When one of the three state-changing functions listed above is called on
a *Promise*, callbacks registered to the connected *Future* will be
invoked. You can control whether this invocation is made synchronously,
or asynchronously using a thread from an internal thread pool, by passing
one of *FutureCallbackType_Sync* and *FutureCallbackType_Async* to the
*Promise* constructor.


qi::FutureSync
==============
.. _api-futuresync:

:cpp:class:`qi::FutureSync` is a lightweight wrapper on top of
:cpp:class:`qi::Future` that will wait on the *Future* in its destructor
if the *Future* was ignored by the user.

It is intended to be used as a way to provide a default apparent
synchronous-blocking behavior to a function, that can be changed into
an asynchronous behavior by handling the resulting *FutureSync*.

Returning a FutureSync
----------------------

You can simply change the returned type from *Future* to *FutureSync* in the
::ref:`basic example <api-promise>`: The returned *Future* will transparently
convert to a *FutureSync*.

Calling a function returning a FutureSync
-----------------------------------------

*FutureSync* follow this simple rule: The destructor will call
:cpp:func:`qi::Future<T>::wait` from its destructor, unless:

- It is copied into another *Future* or *FutureSync*
- :cpp:func:`qi::FutureSync<T>::async` or any of the Future function is called (*wait*, *connect*, ...)

.. code-block:: cpp

  qi::FutureSync<int> someFunction();
  void test()
  {
    someFunction(); // will wait
    qi::FutureSync<int> f = someFunction(); // will wait at end of scope
    someFunction().async();                 // will not wait
    qi::Future<int> f = someFunction();     // will not wait
    someFunction().value();                 // will wait, because of value()
  }


.. cpp:autoenum:: FutureState

.. cpp:autoclass:: qi::Future



.. cpp:autoclass:: qi::Promise


.. cpp:autoclass:: qi::FutureSync

