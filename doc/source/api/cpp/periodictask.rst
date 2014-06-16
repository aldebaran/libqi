.. _api-periodictask:
.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::PeriodicTask API
********************

Introduction
------------

qimessaging provides numerous ways of doing async operations, but sometimes you
need an async operation which repeats itself. This is what you can do with
`qi::PeriodicTask`.

Using a periodic task
---------------------

.. code-block:: cpp

  void printDate();

  qi::PeriodicTask task;
  task.setName("print date");
  task.setUsPeriod(1000*1000);
  task.setCallback(printDate);
  task.start();

  // and at the end
  task.stop();

This will create a task which will call printDate every second. The name of the
task is not used yet, but may be useful to provide debug information later.
`start()` will call the task immediately (unless given false as argument) and
call it again every second.

In all cases, `PeriodicTask` never gives hard time guaranties (like realtime).
If there is a lot of work in the event loop, your task may be called later than
expected.

However, `PeriodictTask` guaranties you that it will *never* call your callback
twice at the same time, even if it missed it's deadline. Also, when you call
stop, it will block until the callback is finished if it's currently running.

About scheduling and callback time compensation
-----------------------------------------------

Sometime, your callback may be slow and you may wonder what happens in these
cases.

Here is a simple task with a 5s period and a 3s callback::

  v                                v
  +-----------+                    +-----------
  +  Task 3s  +      wait 5s       +  Task 3s  ...
  +-----------+--------------------+-----------

Same thing with callback compensation enabled::

  v                     v
  +-----------+         +-----------
  +  Task 3s  + wait 2s +  Task 3s  ...
  +-----------+---------+-----------

Same thing again with a slower task::

  v                            v
  +----------------------------+---------------------------+----
  +         Task 7s            +         Task 7s           +  ...
  +----------------------------+---------------------------+----

The task will never be called twice even if it takes longer than the period.

Note that it is not recommended to use a periodic task with slow tasks (as
other qimessaging's async methods).

.. cpp:autoclass:: qi::PeriodicTask
