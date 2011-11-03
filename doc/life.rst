NAO's life
==========

Sentinel
--------

What to do about sentinel?

Gathering information:

send signals for temperature, buttonclick, etc... => SystemManager, Sensors, ...

=> react to signal:
  chestbutton => system menu
  motorhot    =>


NAO's life core:

only do basic stuff and give the hand to the decision core.

React to global event:
 - chestbutton
 - motorhot


.. _life-remote-app:

Remote Application
------------------

How remote application interact with the robot?

When an application like Choregraphe connect to the robot, the robot should ask the user if he want choregraphe to connect to the robot, the robot can remember the choice for future connection. Application can be on hold til the robot return to a idle state, or could stop the current application. A permission allow the remote apps to stop the current robot activity.
