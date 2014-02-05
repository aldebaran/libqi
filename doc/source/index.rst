.. role:: cpp(code)
   :language: cpp

.. role:: bash(code)
   :language: bash

Documentation of qi::log
************************

Introduction
============

Header: :cpp:`<qi/log.hpp>`

Overview
========

Log are hierarchical with "." as separator. Each level of hierarchy is called a
**category**.

For example, for an instant messaging service, categories may be:

.. code::

  im.audio
  im.audio.input
  im.audio.output
  im.audio.internal
  im.video
  im.video.internal
  im.http

This allows easy log filtering in external log tools.

There are 7 levels of log:

:silent: Hide logs.

:fatal: Used before the program exit.

:error: A classical error.

:warning: Useful to warn user.

:info: Useful to user information.

:verbose: Not mandatory but useful to user information.

:debug: Useful to developer. Not compile on release.

Writing logs
============

There is a log function for each level except **silent**. Those functions are
basic streams.

.. code-block:: c++

  std::string msg = "Message to print with number: ";
  qiLogFatal("ca.te.go.ry")   << msg << 1;
  qiLogError("ca.te.go.ry")   << msg << 2;
  qiLogWarning("ca.te.go.ry") << msg << 3;
  qiLogInfo("ca.te.go.ry")    << msg << 4;
  qiLogVerbose("ca.te.go.ry") << msg << 5;
  qiLogDebug("ca.te.go.ry")   << msg << 6;

.. note::

  Do not add enlines manually, qi::log automatically adds them at the end of
  each message.

To avoid typing category each time (and so make mistakes), there is a scope
function to set it globally: :cpp:`qiLogCategory(const char* category);`

.. note::

  It is always allowed to set a specific category for a message even if a global
  category is set.

.. code-block:: c++

  qiLogCategory("ca.te.go.ry")

  qiLogFatal()   << "foo";
  qiLogError()   << "foo";
  qiLogWarning() << "foo";
  qiLogInfo()    << "foo";
  qiLogVerbose() << "foo";
  qiLogDebug()   << "foo";

.. warning::

  If log category is set globally in an include file, all files including this
  file will have this category as global category.

Filtering logs output
=====================

It is possible to change the output of log in 3 different ways:

- category,
- level,
- context.

Level
+++++

**level** via :bash:`QI_LOG_LEVEL` environment variable.

If :bash:`QI_LOG_LEVEL` is set to :bash:`"fatal"` only **fatal** logs are
displayed.

if :bash:`QI_LOG_LEVEL` is set to :bash:`"info"`, **fatal**, **error**,
**warning** and **info** logs are displayed.

.. note::

  By default, level is set to **info**.

It is possible to use corresponding number instead of name of levels.

0. Silent
1. Fatal
2. Error
3. Warning
4. Info
5. Verbose
6. Debug

Context
+++++++

**context** are the meta information collected during the creation of log.

These information are:

- Level,
- Date,
- ThreadId,
- Category,
- File,
- Function.

To set context use :bash:`QI_LOG_CONTEXT`, which is a bit-field.

:1: Level as complete string
:2: Level as a single letter
:4: Date of emission
:8: ThreadId
:16: Category
:32: File name and line number
:64: Function name
:128: End of line

Useful values of contexts:

:26: Short level + threadId + category
:30: Short level + threadId + date + category
:126: Short level + threadId + date + category + file + function
:254: Short level + threadId + date + category + file + function + eol

Category filtering
++++++++++++++++++

**category** can be filtered via :bash:`QI_LOG_FILTERS` environment variable.

**-** is used to remove a category, **+** to add it, **:** is the separator.

.. code-block:: bash

  QI_LOG_FILTERS="-im.audio:+im.video"

:bash:`QI_LOG_FILTERS` support globbing.

.. code-block:: bash

  QI_LOG_FILTERS="-im*:+im.video*"

It is possible via :bash:`QI_LOG_FILTERS` to set different level of verbosity
for each category via **=**.

.. code-block:: bash

  # set level of verbosity to warning for im* except for im.video to debug.
  QI_LOG_FILTERS="+im*=3:+im.video=6"

.. warning::

  Due to this feature, use :bash:`QI_LOG_LEVEL` with :bash:`QI_LOG_FILTERS` may
  be hazardous.

