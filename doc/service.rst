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
