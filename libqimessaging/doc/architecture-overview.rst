QiMessaging global architecture overview
========================================

Abstract
--------

This document describes the global architecture of QiMessaging from a network
point of view.

Principle
---------

There are five layers to QiMessaging architecture:
- Client
- Connection
- Gateway
- ServiceDirectory
- Service

When a computer wants to use a service, it needs to instanciate a connection on
the client side.

 +------------+
 | Connection |
 +------------+

Then instanciate a Client and connect it to the Connexion.

 +------------+
 |   Client   |
 +------------+
 | Connection |
 +------------+

When the Client requests a service, a socket will be establish between
Connection and the Gateway on the robot.

 +------------+
 |   Client   |
 +------------+
 | Connection |
 +------------+
       |        client
 ------|--------------
       |         robot
 +------------+
 |  Gateway   |
 +------------+

The Gateway will then interrogate the ServiceDirectory to get the address of
the Service.

 +------------+
 |   Client   |
 +------------+
 | Connection |
 +------------+
       |        client
 ------|--------------
       |         robot
 +------------+
 |  Gateway   |
 +------------+
       |
 +------------+
 | ServiceDir |
 +------------+

Once the Gateway has the service address, it will associate a the connection
from the client to this service and messages will be forwarded.

 +------------+
 |   Client   |
 +------------+
 | Connection |
 +------------+
       |        client
 ------|--------------
       |         robot
 +------------+
 |  Gateway   |
 +------------+--------+
                       |
 +------------+  +------------+
 | ServiceDir |  |  Service   |
 +------------+  +------------+

One Client can only be connected to one Service.

Multiple Clients
----------------

 +------------+------------+ +------------+------------+
 |   Client   |   Client   | |   Client   |   Client   |
 +------------+------------+ +------------+------------+
 |       Connection        | |       Connection        |
 +-------------------------+ +-------------------------+
             | |         client          | |
 ------------|-|-------------------------|-|------------
             | |          robot          | |
 +-----------------------------------------------------+
 |                        Gateway                      |
 +-----------------------------------------------------+
        |            | |             |            |
 +------------+ +------------+ +-----------+ +---------+
 | ServiceDir | |  Service   | |  Service  | | Service |
 +------------+ +------------+ +-----------+ +---------+

Far-futur improvements
----------------------

Multiplex between one Connection and the Gateway?
