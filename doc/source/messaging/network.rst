.. _qimessaging-networking:

qiMessaging - Networking
========================

Please read the :ref:`use case <qimessaging-usecase>` to understand the
requirement we have.

Networking
----------

We need to respond to three major differents communication typse with differents
capabilites (security level, acces points, ...):

We support multiple communications ways. We need a way to select which
communication's means we should use.

.. image:: /medias/qimessaging-networking.png

A service can have multiple endpoints that represent a way to connect them, for
example a service can support ipc and tcp endpoints. To connect client to their
final service we need a ServiceDirectory which enumerate services and endpoints
associated to them.

IPv4 vs IPv6
------------

IPv4 and IPv6 are supported, but you need to know that you cannot connect from
one type to another. For example, if you listen on an IPv4 address you can only
connect to it with the same IPv4 address. same rule applies for IPv6.

If you want to bind on all IPs you must listen on both tcp://0.0.0.0:0 for IPv4
and tcp://::0:0 for IPv6 addresses.

If you set the port to 0 (e.g. tcp://127.0.0.1:0 or tcp://::1:0),
the system will choose a valid and available port to listen to.

If you want to use IPv6 addresses, you must know that the scope identifier is
hardcoded to the name of the hardware interface associated with the address (such as
eth0). Example would be like "fe80::1%eth0", which means "fe80::1y on the
link associated with eth0 interface".

For more explanation about IPv6, you can read `IPv6 on wikipedia`_.

.. _IPv6 on wikipedia: http://en.wikipedia.org/wiki/IPv6

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



.. _qimessaging-internal_network:

Direct connection protocol
--------------------------

Communications are done inside the robot, this implies few things:

* do not need a lot of security level, all communications are known and we
  assure they are safe,
* they can be some acces points to connect to:

  * LPC (Local Procedure Call): communication are done on 1 machine (single
    computer robots) and 1 process,
  * IPC (Inter Process Communication): communication are done on 1 machine but
    between multiple processes,
  * RPC (Remote Process Communication): communication are done between multiple
    machines (multiple computers robots).

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

  * communication are done between multiple machines
    (multiple computers robots).
  * There is the differents machineID and processID.
  * Communication through tcp/ip, ssl (not really usefull since we manage
    those connections), any other backend that allow remote connection.

.. _qimessaging-external_network:

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

more information on :ref:`qimessaging-gateway`.


TCP/IP Gateway
^^^^^^^^^^^^^^

The Transmission Control Protocol (TCP) and Internet Protocol (IP) is used for
LAN connection:

* Choreagraphe to Robot
* Remote devices on the same subnet than the robot
* Internal connection

Have a look at :ref:`qimessaging-gateway` for more technical details.

.. image:: /medias/network_with_tcp_gateway.png

XMPP:
^^^^^

To acces to the robot from everywhere, we use Extensible Messaging and Presence
Protocol (XMPP). The XMPP network uses a client-server architecture (clients do
not talk directly to one another). However, it is decentralized-by design, there
is no central authoritative server. Every user on the network has a unique ID.

Another useful feature of the XMPP system is that gateways allow users to access
networks using other protocols. XMPP provides this access at the server level
by communicating via special gateway services running on a remote computer. Any
user can "register" with one of these gateways by providing the information
needed to log on to that network, and can then communicate with users of that
network as though they were XMPP users. This means any client that fully
supports XMPP can access any network with a gateway without extra code in the
client, and without the need for the client to have direct access to the
Internet.


* Weaknesses

  * *In-band binary data transfer is inefficient*
    Because XMPP is not yet encoded as `Efficient XML Interchange`_  but as a
    single long XML document, binary data must be first base64 encoded before it
    can be transmitted in-band. Therefore any significant amount of binary data
    (e.g., file transfers) is best transmitted out-of-band, using in-band
    messages to coordinate.

    .. _Efficient XML Interchange: http://en.wikipedia.org/wiki/Efficient_XML_Interchange


* Strengths

  * Decentralization
  * Open standards
  * Security:

    XMPP servers may be isolated from the public XMPP network (e.g., on a
    company intranet), and robust security (via SASL and TLS) has been built
    into the core XMPP specifications. To encourage use of channel encryption,
    the XMPP Standards Foundation currently runs an intermediate certification
    authority at StartSSL (formerly at xmpp.net) offering free digital
    certificates to XMPP server administrators under the auspices of the
    StartCom Certification Authority (which is the root CA for the intermediate
    CA).

  * Flexibility:

    Custom functionality can be built on top of XMPP; to maintain
    interoperability, common extensions are managed by the XMPP Software
    Foundation. XMPP applications, network management, content syndication,
    collaboration tools, file sharing, remote systems control and monitoring,
    geolocation, middleware and cloud computing, VoIP and Identity services.

.. image:: /medias/xmpp_gateway.png

.. warning::

  * Server performance for lots of client?
  * Token to identify robots?







