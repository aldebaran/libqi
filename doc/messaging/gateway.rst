.. _qimessaging-gateway:

qiMessaging - Gateway
=====================

.. warning::

  - threading
  - security (master isolated from gateway)
  - multiple client
  - describe the event loop / how does it works
  - particularity of xmpp/secure

Overview
--------

.. warning::

  schema: NAOqi 2.0 - Gateway (secure/xmpp/ros/yarp)

Internal Machinery
------------------

A TransportClient can connect to a remote Service. he create two streams (one in each way).
This is how he communicate with a service.
Message are send through one pipe and receive though the other. Messages are typed "methodcall", "stream"

.. image:: /medias/network_machinery.png


