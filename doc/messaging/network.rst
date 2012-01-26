.. _qimessaging-networking:

qiMessaging - Networking
========================

Use Case
--------

.. warning::

  schema from http://doc/continuous/specification/spec/romeo/network.html

  explain differences between external and internal networking
  nat traversal and proxy traversal issues


We need to respond to two major differents communication type with differents capabilites (security level, acces points, ...):

**Internal: Direct connection protocol**

Communications are done inside the robot, this implies few things:

* do not need a lot of security level, all communications are known and we assure they are safe,
* they can be some acces points to connect to:

  * LPC (Local Procedure Call): communication are done on 1 machine (single computer robots) and 1 process,
  * IPC (Inter Process Communication): communication are done on 1 machine but between multiple processes,
  * RPC (Remote Process Communication): communication are done between multiple machines (multiple computers robots).


**External: Remote connection protocol**

* Remote Device

  * Remote device that could be used by a robot.


Networking
----------

We support multiple communications ways. We have need a way to select which communication means we should use.

.. image:: /medias/qimessaging-networking.png

A service can have multiple endpoints that represent a way to connect them, for example a service can support ipc and tcp endpoints.
To connect client to their final service we need a ServiceDirectory which enumerate services and endpoints associated to them.

ServiceDirectory
----------------

Because we have multiple possible endpoint we have a service that is used only to connect client and services between them. It's the central point.

.. warning::

  schema service directory

more information on :ref:`qimessaging-servicedirectory`.

Direct connection protocol
--------------------------

Those endpoint's protocols are only used on the same device. A device can be an agregation of multiples computers.
But they are connected on the same internal network bus. (external network bus should be separated from the internal one).
Those method do not need to use a gateway, they connect to the ServiceDirectory to get the service address then a direct connection to the service is established.

.. image:: /medias/network_without_gateway.png

LPC:

  When: same machineId and processId.
  pointer on the service or proxy object with pointers.

IPC:

  When: same machineId

RPC:
  When: different machineId


Remote connection protocol (Gateway)
------------------------------------

To connect to a robot using the external network interface of the device, we support a tcp/ip gateway that can use SSL, and an XMPP to connect using a third party server. The gateway allow to support NAT and proxy traversal. Futhermore it allow to apply specific security/permission to all external connection.

more information on :ref:`qimessaging-gateway`.


TCP/IP Gateway
^^^^^^^^^^^^^^

.. image:: /medias/network_with_gateway.png

Have a look at :ref:`qimessaging-gateway` for more technical details.

XMPP:
^^^^^
When:

.. warning::

  XMPP gateway
  schema (herve)
  and blabla. talk about presence. each device announce their device presence on the xmpp server.
  token to identify robots?







