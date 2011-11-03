XAR
===

XML Aldebaran Robotics.

A XAR is a file that describe a Service, a Box, a TimeLine, ...

XAR have two basic fonctionnality:
  - describe our API
  - describe how objects are connected


basic types:

.. code-block:: xml

  <service>
    <input name=""/>
    <output name=""/>
    <parameter name=""/>
    <connect in="toto" out="titi"/>
  </service>

XAR can be used by our "moc" to generate information about a service, a box, a behavior, a filter...

two parts: the client part, that mostly follow alproxies scheme. the server part that mostly follow "moc".

TML
===

Timeline XML

describe a timeline.

MOC
===

What need to be generated?

  => reflection information about inputs, outputs, parameters, methodcalls, custom struct ...
  => serialization code
