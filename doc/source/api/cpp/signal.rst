.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

.. _api-signal:
Signal Documentation
********************

qi::Signal<T>
=============


.. cpp:class:: qi::Signal

.. code-block:: c++

  #include <qitype/signal.hpp>


Implementation of the *signal/event* paradigm, with some specificities:

- Thread-safe.
- Synchronous or asynchronous subscriber invocation.
- Subscriber disconnection garantees that subscriber is not/will not be called
  anymore when it returns.
- Automatic subscriber disconnection when *weak_ptr* or *Trackable* is used.

`qi::Signal` has pointer semantics: copies of a `Signal` share the same state.


Template argument
-----------------

`Signal` is templated by the argument types that must be passed when triggering,
and that will be transmitted to subscribers. For instance *Signal<int, int>*
is the type of a *Signal* with two ints as payload type: `Signal::operator()`
will expect two ints, and subscribers will be expected to have signature *void(int, int)*.

Subscribing to the signal
-------------------------

.. cpp:function:: SignalSubscriber& Signal::connect(function, ...)

Use `Signal::connect` to register a subscriber that will be called each time
the signal is triggered.

Arguments
~~~~~~~~~

Arguments to *connect* can take multiple forms:

- A function or callable object (like *boost::function*).
- A function or callable, followed by arguments and placeholders that will be
  bound to the function (more about that below).
- An other compatible `Signal`.

The variadic form of *connect* works in a similar manner to *boost::bind()*:
values passed to *connect* will be passed to the function, in order, and
*placeholders* *_1*, *_2* ... will be replaced by the signal payloads.

This form will also recognise if the first argument is a *boost::weak_ptr*, or
if it as pointer to a class inheriting `qi::Trackable`. In both cases, the
subscriber will be automatically disconnected if the pointer cannot be locked.
See :ref:`this example<future-connect>` for a demonstration of that very same
mechanism in `qi::Future`.

Return value
~~~~~~~~~~~~

.. _signal-setCallType:

`Signal::connect` returns a *SignalSubscriber&*, that you can use to:

- Override the default call type for this subscriber to synchronous or asynchronous
  by calling `SignalSubscriber::setCallType`.
- Obtain a subscriber identifier of type `qi::SignalLink` by casting the `SignalSubscriber`:

.. code-block:: c++

  qi::SignalLink l1 = someSignal.connect(callback1);
  qi::SignalLink l2 = someSignal.connect(callback2).setCallType(qi::MetaCallType_Direct);


Unregistering a subscriber
--------------------------

.. cpp:function:: void Signal::disconnect(qi::SignalLink subscriberId)

Unregistering a subscriber is done by invoking `Signal::disconnect` with a
`SignalLink` as its sole argument. The call will block until all currently
running invocations of the subscriber have finished. This gives you the strong
garantee than once *disconnect* has returned, your callback function is not being
called, and will never be called again.


Triggering the signal
---------------------

.. cpp:function:: void Signal::operator()(T)

Trigger the signal is achieved by using the `Signal::operator()`, with
arguments matching the `Signal` type:

.. code-block:: c++

  qi::Signal<int, int> sig;
  sig(51, 42);

This will invoke all subscribers with given arguments.

.. cpp:function:: void Signal::setCallType(MetaCallType callType)

Controls how subscribers are invoked:

- `MetaCallType_Auto` is the default and means asynchronous.
- `MetaCallType_Direct` forces a synchronous call.
- `MetaCallType_Queued` forces an asynchronous call.


Note that if any subscriber is invoked asynchronously, the arguments passed to
`Signal::operator()` will be copied.

Monitoring the presence of subscribers
--------------------------------------

.. cpp:function: Signal::Signal(boost::function<void(bool)> onSuscrbiers)

Sometimes, mainly for performance reasons, it is useful to only enable some
code if a `Signal` has at least one subscriber. This can be achieved by
passing a callback to the *Signal* constructor, of signature *void(bool)*.
This function will be called each time the number of subscribers switches
between 0 and 1.

Overriding the default Signal behavior
--------------------------------------

.. cpp:function: Signal::setTriggerOverride(Trigger trigger)
.. cpp:function: Signal::callSubscribers(const GenericFunctionParameters args, MetaCallType callType)

Sometimes, mainly when bridging `Signal` with an other signal implementation, one
needs to override the action performed when the signal is triggered (which is
by default to invoke all subscribers).

This can be achieved by inheriting from `Signal`, and then either overriding the
`Signal::trigger` virtual function, or calling `Signal::setTriggerOverride` with
a functor that will replace the original trigger. You can then call
`Signal::callSubscribers` to invoke the subscribers, which *trigger* would do
by default.

.. cpp:class:: qi::SignalSubscriber

.. cpp:function: SignalSubscriber::setCallType(MetaCallType)

Set the call type used for this subscriber. If set to `MetaCallType_Auto`,
the call type set for the signal (by `Signal::setCallType` will be used.
