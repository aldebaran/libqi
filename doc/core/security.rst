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


Implementation (WIP)
--------------------

ApplicationManager know permissions of applications. A token is associated to a set of permissions. A token can be associated to a local or remote apps.

Remote Application should be "installed" on the robot, before being able to communication with a robot.

Each services ask (and cache) ApplicationManager about application who connect to know if they have specific permissions...

Broker authenticate to a ServiceDirectoryManager using an authentication token.

this auth token represent a remote client with a specific set of permissions (only known by the robot). this token is used everywhere to identify the client.

some permissions are "remote", handled by servers. some permissions are "local", handled by the system. (write access, ...)


