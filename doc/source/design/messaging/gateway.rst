.. _qimessaging-gateway:

qiMessaging - Gateway
=====================

.. warning::

  - multiple client
  - particularity of xmpp/secure

Overview
--------

.. image:: /medias/NAOqi-2.0-Gateway-secure-xmpp-ros-yarp.png


Event Loop
++++++++++

The event loop or message dispatcher, is a programming construct that waits for and dispatches events or messages in a program.

It works by polling some internal or external "event provider", which generally blocks until an event has arrived, and then calls the relevant event handler ("dispatches the event").

The event loop almost always operates asynchronously with the message originator.

Asynchronous
++++++++++++

Asynchronous events are those occurring independently of the main program flow. Asynchronous actions are actions executed in a non-blocking scheme, allowing the main program flow to continue processing before the transmission has finished.

Asynchronous method dispatch (AMD) is a data communication method used when there is a need for the server side to handle a large number of long lasting client requests. Using synchronous method dispatch (SMD), this scenario may turn the server into an unvavailable busy state resulting in a connection failure response caused by a network connection request timeout.

The servicing of a client request is immediately dispatched to an available thread from pool of threads or threads (:ref:`Network Thread <qimessaging-network_thread>`). Upon the completion of the task, the server is notified by a callback. The server notifies the client and transmits the response back to the client.

Synchronous
+++++++++++

.. warning::

  We want an easy way to switch between asynchronous and synchonous communication to make debuging easier?

.. _qimessaging-network_thread:

Network Thread
++++++++++++++

A Network Thread will dispatch the events coming from everywhere. You can work with pool thread or self created threads into your process. You need them each time you create a Client or Server, it will allow to work asynchronously.

.. warning::

  Multiple Client and Server?


Transport Server
++++++++++++++++

Transport Client
++++++++++++++++






Internal Machinery
------------------

A gateway is a network point that acts as an entrance to another network, in our case the :ref:`external <qimessaging-external_network>` and :ref:`internal <qimessaging-internal_network>` network. A gateway acts as a portal between those two programs allowing them to share information by communicating between protocols though that gateway.

To forward messages from external to internal network, the gateway work closely with the :ref:`Service Directory<qimessaging-servicedirectory>`.

The gateway could be in the same process or isolated from the :ref:`Service Directory<qimessaging-servicedirectory>`.

.. warning::

  Security issue?


A gateway has two differents behavior:

  - it works as a Server for anyone how want to connect to it, or more exactly to connect to a Service
  - it works as a Client to connect external Client and Service together.



A TransportClient can connect to a Service. It create two streams (one in each way).
This is how he communicate with a service.
Message are send through one pipe and receive though the other. Messages are typed "methodcall", "stream"

.. image:: /medias/network_machinery.png


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

