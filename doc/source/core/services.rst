Core Services
=============

This describe all basic services provided by the NAOqi framework. They could be implemented on any devices.

LogManager
----------

Could be used to get a stream of logs. The stream could be tweaked by each clients, they set verbosity and filters on category.

.. code-block:: python

  class LogInput
     Param: verbosity, filters
     signal: onLog, onFinished
     slot: start, stop

  class LogOutput
     slot: log(verbosity, cat, message)

  LogManager
    LogInput  logInput()
    LogOutput logOutput()


SettingsManager
---------------

Could be used to get/set the *user* preferences of the Robot. This include system preferences (system, audio, video, motion, ..) and application preferences.

.. code-block:: python

  class SettingsManager:
    param: ?
    List groupList()
    settingsMap(groupname)

    setSetting(groupname, name, value);
    Value setting(groupname, name);


SystemManager
-------------

Manage the system: volume, name, language, mynao credentials

NetworkManager
--------------

network settings. Allow to retrive the list of connected networks, the list of available networks, the list of known networks

class NetworkInfo:
  class IpInfo:
    method  = string()
    address = string()
    netmask = string()
    gateway = string()

send events:
 - network/online
 - network/offline
 - network/statusChange


PackageManager
--------------

list installed package, retrive manifest, retrieve preferences, install/remove package.
Retrieve package from the store as needed.

ServiceManager
--------------

start/stop services

BehaviorManager
---------------

start/stop behaviors ?

to merge into a single ApplicationManager?

DeviceManager
-------------

set/get the device status, set/get ressources, device type, device topology ?

NotificationManager
-------------------

notification for the robot. May be integrated in the system menu?


Context
-------

provide reflection informations : method calls, commands, ...
