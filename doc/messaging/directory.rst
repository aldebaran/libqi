.. _qimessaging-servicedirectory:

qiMessaging - ServiceDirectory
==============================

ServiceDirectory
----------------

Only service on lpc/ipc/rpc can be registered. It should not be possible to register service behind a gateway nor XMPP.
Clients connected using the gateway will continue using the same gateway whatever happend.
The endpoint selection mecanism apply only to client on on at least the same subnetwork.

All clients who want to connect to a service could ask the ServiceDirectory about service location and available endpoints.
When external clients connect to a gateway, it's the gateway role to handle all lookup, in that case services will have internal endpoint not accessible directly from the client, and the gateway will take care of everything for the client.

.. warning::

  - what does it store
  - how is it related to gateway
  - how to register a service
  - API
  - which services can be registered
  - one by board? one by robot?




