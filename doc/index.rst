.. qi Framework documentation master file, created by
   sphinx-quickstart on Fri Oct 21 12:29:28 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to qi Framework's documentation!
========================================


Overview
--------

This documentation is a work in progress. Please comments, modify :)


Take a look at the :ref:`global architecture overview <architecture>`

Standards
---------

 - :ref:`Binary Message <std-binary-message>`:  describe the network binary network protocol used in qiMessaging.
 - :ref:`std-code-convention` : code convention

Topics
------

 - :ref:`qiMessaging <qimessaging-index>`
 - :ref:`NAOqi 2.0 - Core <core-index>`

Software Stack
--------------

.. image:: /medias/NAOqi2.0-Softwarestack.png



.. toctree::
   :maxdepth: 2
   :hidden:

   architecture
   std/index
   messaging/index
   core/index

..
   connectivity
   application
   choregraphe
   core
   tracing
   xar
   security
   threading
   messaging
   life
   animation
   manifest
   generator

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`














..
   std/xar
     Describe connection between objects

   std/tml
     Timeline XML

   std/manifest
     Describe application content

   std/aml
     API XML
     describe service, methods, signal, slot, events, ..

   std/metadata
     describe app, used by nao's life and naostore.
     (can be edited online)


..
  Plan

  global overview
    architecture
    messaging
    choregraphe

  sharing
    connectivity
      remote application, google apps?,

    application
    NAOStore
    NAO's life

  core:
    core services
    API

  programming
    cpp: using xar files generate header and sources for reflections
    python: reading xar files
    xar file format description
    manifest file format description
    timeline file format description
    threading
    animation

    tools:
      - ide to write code in python and c++
      - xar editor: edit API (objects, services, methods, properties, events?)
      - timeline editor
      - choregraphe: the full package :)
         - xareditor, timelineeditor, metadataeditor, ressourceeditor, ..
         - oscilloscope mode : inspect all signals (streams, events, parameters, ..)
         - gather and display statistics about service's usage, trafic, latency, ..
      - representation of the robot, ik3d


  security
  tracing


  TODO:
  - how to support simple method call in choregraphe? (support threaded slot?)

  a method call is a threaded slot?


  Graph:
    - Application ecosystem (computer, tablet, embedded)
    - connectivity
    - core overview: basic services
    - application architecture: service, behavior, threads, shared memory


