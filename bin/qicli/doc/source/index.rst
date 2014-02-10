Documentation of qicli
**********************

Introduction
============

There is several sub-commands:

* :ref:`info<subcmd-info>`: give information about services.
* :ref:`call<subcmd-call>`: make a call on a service's method.
* :ref:`post<subcmd-post>`: send a signal.
* :ref:`get<subcmd-get>`: get a property.
* :ref:`set<subcmd-set>`: set a property.
* :ref:`watch<subcmd-watch>`: watch service's signals.
* :ref:`top<subcmd-top>`: documentation will come
* :ref:`trace<subcmd-trace>`: documentation will come
* :ref:`log-view<subcmd-log-view>`: advanced log printer.
* :ref:`log-send<subcmd-log-send>`: send a log.

.. _subcmd-info:

info sub-command
----------------

Main features
+++++++++++++

Either list all services or display detailed information on specific services.

With no argument, it lists services:

.. code-block:: bash

   $ qicli info
   > 001 [ServiceDirectory]
     002 [LogManager]
     003 [ALFileManager]
     004 [ALMemory]
     005 [ALLogger]
     ...

This is equivalent to:

.. code-block:: bash

   $ qicli info --list
   $ qicli info -l
   $ qicli # Without any argument


With service's name as argument, it gives:

.. code-block:: bash

   $ qicli info ServiceDirectory
   > 001 [ServiceDirectory]
     * Info:
      machine   37814cee-e5a8-4183-9862-65d10460f0e5
      process   3665
      endpoints tcp://127.0.0.1:9559
                tcp://198.18.0.1:9559
                tcp://10.2.1.177:9559
     * Methods:
      100 service            (String)
      101 services          List<> ()
      102 registerService   UInt32 ()
      103 unregisterService Void (UInt32)
      104 serviceReady      Void (UInt32)
      105 updateServiceInfo Void ()
      108 machineId         String ()
     * Signals:
      106 serviceAdded   (UInt32,String)
      107 serviceRemoved (UInt32,String)

There is always 3 parts:

**Info**
  give general information about service,
**Methods**
  list of methods,
**Signals**
  list of signals.

The 2 last parts may be empty.

**Methods** and **Signal** follow the format::

   [id] name    [return_type] ([parameters_types...])

If the name of the service is a bit long, identifiers can be used instead:

.. code-block:: bash

   $ qicli info ServiceDirectory
   $ qicli info 1

Command also accept several service's names simultaneously:

.. code-block:: bash

   $ qicli info ServiceDirectory LogManager
   > 001 [ServiceDirectory]
       * Info:
        machine   37814cee-e5a8-4183-9862-65d10460f0e5
        process   3665
        endpoints tcp://127.0.0.1:9559
                  tcp://198.18.0.1:9559
                  tcp://10.2.1.177:9559
       * Methods:
        100 service            (String)
        101 services          List<> ()
        102 registerService   UInt32 ()
        103 unregisterService Void (UInt32)
        104 serviceReady      Void (UInt32)
        105 updateServiceInfo Void ()
        108 machineId         String ()
       * Signals:
        106 serviceAdded   (UInt32,String)
        107 serviceRemoved (UInt32,String)
     002 [LogManager]
       * Info:
        machine   37814cee-e5a8-4183-9862-65d10460f0e5
        process   3665
        endpoints tcp://127.0.0.1:9559
                  tcp://198.18.0.1:9559
                  tcp://10.2.1.177:9559
       * Methods:
        100 log            Void (LogMessage)
        101 getListener    Object ()
        102 addProvider    Int32 (Ob ject)
        103 removeProvider Void (Int32)
       * Signals:

Globing can also be used:

.. code-block:: bash

   $ qicli info "LogMa*"
   > 002 [LogManager]
       * Info:
        machine   37814cee-e5a8-4183-9862-65d10460f0e5
        process   3665
        endpoints tcp://127.0.0.1:9559
                  tcp://198.18.0.1:9559
                  tcp://10.2.1.177:9559
       * Methods:
        100 log            Void (LogMessage)
        101 getListener    Object ()
        102 addProvider    Int32 (Object)
        103 removeProvider Void (Int32)
       * Signals:

Advanced features
+++++++++++++++++

Extra options:

**--show-doc**
  display documentation if available.

**--hidden**
  also display hidden services or methods/signals.

.. _subcmd-call:

call sub-command
----------------

This command allows to call service's methods.

Return values follow `json format <http://www.json.org/>`_.

.. code-block:: bash

   $ qicli call ALFileManager.ping
   > ALFileManager.ping: true

More complex return values can also be returned.

.. code-block:: bash

   $ qicli call ALMemory.getEventList
   > ALMemory.getEventList: [ "/Preferences", "ALAnimatedSpeech/EndOfAnimatedSpeech", "ALAudioSourceLocalization/SoundLocated", "ALAudioSourceLocalization/SoundsLocated", ...]

.. note::

   [ ... ] is a list in json.

Identifiers can also be used instead of names:

.. code-block:: bash

   $ qicli call 4.123
   > ALMemory.getEventList: [ "/Preferences", "ALAnimatedSpeech/EndOfAnimatedSpeech", "ALAudioSourceLocalization/SoundLocated", "ALAudioSourceLocalization/SoundsLocated", ...]

Several arguments can be given:

.. code-block:: bash

   $ qicli call ServiceDirectory.service PackageManager
   > ServiceDirectory.service: [ "PackageManager", 41, "4fd62363-f74d-4c6d-81d1-c1b9304c77d2", 3840, [ "tcp://10.0.252.216:34510", "tcp://127.0.0.1:34510" ], "0967f415-db38-43a4-b5a9-7ac70539891d" ]

Complex arguments (list, objects) of methods must be given in JSON with option
**--json**.

.. warning::

   JSON and terminals don't work well together.

.. code-block:: bash

   $ qicli call --json ALMemory.insertListData "[[\"foo\",true],[\"bar\",1]]"
   > ALMemory.insertListData: null

In order to call a hidden method, add **--hidden**

.. _subcmd-post:

post sub-command
----------------

.. _subcmd-get:

get sub-command
---------------

.. _subcmd-set:

set sub-command
---------------

.. _subcmd-watch:

watch sub-command
-----------------

.. _subcmd-top:

top sub-command
---------------

.. _subcmd-trace:

trace sub-command
-----------------

.. _subcmd-log-view:

log-view sub-command
--------------------

.. _subcmd-log-send:

log-send sub-command
-------------------

