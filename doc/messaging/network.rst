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

We support multiple communications ways. We need a way to select which
communication's means we should use.

.. image:: /medias/qimessaging-networking.png

A service can have multiple endpoints that represent a way to connect them, for
example a service can support ipc and tcp endpoints. To connect client to their
final service we need a ServiceDirectory which enumerate services and endpoints
associated to them.

Service Directory
-----------------

A directory service is the software system that stores, organizes and provides
access to resources informations (address, port, connection type - xmpp, tcp/ip,
ssl, socks).

Because we have multiple possible endpoint on a device (a computer or an
agregation of multiples ones), we have a service directory that is used only
to connect client and resources together, on the same internal network bus.
With the name of the service, a user can communicate with a service without
knowing the real physical address of a network resource.

Providing a name to the service directory, anyone will locate the resource.
Each resource on the network is considered an object on the directory server.
A service must register itself to the service directory providing informations
to connect to it.

.. image:: /medias/service_directory.png

more information on :ref:`qimessaging-servicedirectory`.


Direct connection protocol
--------------------------

Those endpoint's protocols are only used on the same device. A device can be an
agregation of multiples computers. But they are connected on the same internal
network bus(external network bus must be separated from the internal one).
Those method do not need to use a gateway, they connect to the
:ref:`Service Directory <qimessaging-servicedirectory>` to get the service
address then a direct connection to the service is established.

.. image:: /medias/network_without_gateway.png


* LPC (Local Procedure Call):

  * Communication are done on 1 machine (single computer robots) and 1 process.
  * There is the same machineID and processID.
  * Easy communication through pointer on the service
    or proxy object with pointers,

* IPC (Inter Process Communication):

  * communication are done on 1 machine but between multiple processes.
  * There is the same machineID but different processID.
  * Communication through tcp/ip, ssl (not really usefull since we manage
    those connections), dbus, any other backend.

* RPC (Remote Process Communication):

  * communication are done between multiple machines (multiple computers robots).
  * There is the differents machineID and processID.
  * Communication through tcp/ip, ssl (not really usefull since we manage
    those connections), any other backend that allow remote connection.

Remote connection protocol (Gateway)
------------------------------------

To connect to a robot using the external network interface of the device,
we support a tcp/ip gateway that can use SSL, and an XMPP to connect using
a third party server. The gateway allow to support NAT and proxy traversal.
Futhermore it allow to apply specific security/permission to all external
connection.

We need to manage multiple computer, with multiple addresses, inside one robot.
We must dispatch each message comming from external network though one address
using gateway and :ref:`Service Directory <qimessaging-servicedirectory>`.

.. image:: /medias/network_with_gateway.png

more information on :ref:`qimessaging-gateway`.


TCP/IP Gateway
^^^^^^^^^^^^^^

Have a look at :ref:`qimessaging-gateway` for more technical details.

XMPP:
^^^^^
When:

.. warning::

  XMPP gateway
  schema (herve)
  and blabla. talk about presence. each device announce their device presence on the xmpp server.
  token to identify robots?







