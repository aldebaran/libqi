.. _api-clock:
.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::Clock
*********

..
   .. cpp:autoclass:: Duration
   .. cpp:autoclass:: NanoSeconds
   .. cpp:autoclass:: MicroSeconds
   .. cpp:autoclass:: MilliSeconds
   .. cpp:autoclass:: Seconds
   .. cpp:autoclass:: Minutes
   .. cpp:autoclass:: Hours

class qi::SteadyClock
=====================

.. cpp:autoclass:: qi::SteadyClock

class qi::WallClock
===================

.. cpp:autoclass:: qi::WallClock

Functions
=========

.. cpp:autofunction:: qi::steadyClockNow
.. cpp:autofunction:: qi::wallClockNow
.. cpp:autofunction:: qi::sleepFor(const qi::Duration&)
.. cpp:autofunction:: qi::sleepUntil(const SteadyClockTimePoint&)
.. cpp:autofunction:: qi::sleepUntil(const WallClockTimePoint&)
