.. _security:

Security
========

.. _local-security:

Application Security
--------------------

Application running on the robot should be constrained in three way:
  - when they have an issue they should have no impact on the system nor on other applications
  - application should declare their permissions and they will be restricted to that
  - application should not bother other applications

Permissions
+++++++++++
An application have a manifest that define what an application is able to do. When calling actions permissions are checked to see if it's possible.

Stability
+++++++++
To have a stable system we divide NAOqi into multiple parts. Each application have it's own process (or many more).
An application can be constitued of services and behaviors.




.. _remote-security:

Remote Application
------------------

Remote application should be handled with care, we cant control them, we dont know how they are made. They should not have access to the whole remote device.
It's very important for them to be restricted by a set of permission.

SessionID
+++++++++
Remote Application should be "installed" on the robot. Installation is a simple matter of defining a manifest with permission associated to a session ID.
When remote clients connect using that sessionID they are given only the permission defined in the manifest.


