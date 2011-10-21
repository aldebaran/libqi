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

