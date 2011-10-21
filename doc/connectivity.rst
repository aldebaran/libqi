.. _connectivity:

Connectivity
============


Global overview:

.. image:: medias/connectivity.png
  :width: 100%



Secure
------

Secure connection are the default to develop with the robot.

Pro:
  - secure

Cons:

xmpp
----

XMPP is the default connection for application that need to connect to NAO from anywhere.

Pro:
  - secure

Cons:


third party gateway
-------------------

ROS, YARP, ...

We can support other robotics framework using insecure gateway.
Should not be enable by default.

Pro:
  - give access to NAO for others robotics framework.

Cons:
  - insecure
  - no control over permission

