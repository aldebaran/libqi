=================================
Path Manipulation Developer Guide
=================================

Overview
--------

For C++ API Reference on logging, go to :doc:`qi::path API Reference </api/namespaces/qipath>`.

The SDK Layout
^^^^^^^^^^^^^^

The main idea of the qibuild-based project is that you always end up with the
same layout. For instance, right after having built your project, you end up with
something like this::

    src
     `+- foo
          `+- data
           |   `+- foo.data
           +- etc
               `+- foo.cfg

    build
     `+- sdk
          `+- lib
           |   `+- libbar.so
           +- bin
               `+- foo

And then when everything is installed, you have something like this::

    prefix
      `+- lib
       |   `+- libbar.so
       +- bin
       |   `+- foo
       +- share
       |   `+- foo
       |        `+- foo.data
       +- etc
           `+- foo
                `+- foo.cfg

The problem
^^^^^^^^^^^

Here is a list of common requirements:

- Find the file foo/foo.cfg, foo/foo.data in a clean, simple way, while making
  sure the solution works whereas the project is built or installed.
- The executable foo may need to write or update its configuration files or data
  but we need to make sure nothing will be written inside the installed directory.
- Since there will be several foo.cfg files, we need to be able to process them
  in a correct order.

The solution
^^^^^^^^^^^^

Here is how it works:

- First we introduce the concept of prefix. When something is built, the prefix
  is /path.to/build/sdk, when something is installed, the prefix is the destination
  directory (``DESTDIR``) plus the installation prefix.
- Then we make the layout in the build prefix and in the install prefix is always
  the same. For instance, we will have CMake rules to be sure that whenever the
  foo project is configured, a copy of foo.cfg is placed in build/sdk/foo/foo.cfg
  (same thing for data)

  - At last, we provide an easy way to get the prefix anywhere from the C++ code.
    The idea is that it is easy to get the prefix from ``argv0``. For instance,
    if ``argv0`` is ``/path/to/build/sdk/bin/foo``, we can assume the prefix is
    ``/path/to/build/sdk/bin/foo``.

Using Namespace qi::path
------------------------

Notes and Requirements
^^^^^^^^^^^^^^^^^^^^^^

When using the API of :doc:`qi::path namespace </api/namespaces/qipath>`, you
should know that:

- Returned paths are absolute and native. (with ``/`` on UNIX and ``\`` on Windows)
- Paths **must** be encoded in UTF-8 encoding.
- All returned paths will be encoded in UTF-8.

For this to work, you must have initialized :cpp:class:`qi::Application` correctly.

Reading and writing configuration files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Writing a configuration file is very different from reading one. Let's assume the
``foo`` executable want to make sure that ``SPAM=42`` in ``foo.cfg``.

Here is how it works:

- First, ask for a **list** of possible paths for ``foo.cfg``.
- Iterate through the list and stop when the first possible ``foo.cfg`` is found.
- Read and update the ``foo.cfg`` file.
- Write the ``foo.cfg`` file.

You can see that we ask for a **list** of paths when reading, but that we always
write to one file.

Let's go through these steps again, assuming foo is installed in ``/usr/bin/foo``,
and ``foo.cfg`` in ``/usr/share/foo/foo.cfg``, and that there is nothing else on
the machine where foo is running.

- First step: ask for a list of possible paths for ``foo.cfg`` using
  :cpp:func:`qi::path::confPaths(const std::string&)`. This gives a list looking
  like: ``[~/.config/foo/foo.cfg, /usr/share/foo/foo.cfg]``
- Since ``.config/foo/foo.cfg`` does not exist, we read ``/usr/share/foo/foo.cfg``
- Then we ask for a location to write: using :cpp:func:`qi::path::confPaths(const std::string&)`.
  In this case it's ``~/.config/foo/foo.cfg``. So we write ``SPAM=42`` to
  ``~/.config/foo/foo.cfg``.

Then each time a piece of code will ask for the ``foo.cfg`` path, it will get a
list starting with ``~/.config/foo/foo.cfg``, so we are sure the setting
``SPAM=42`` will be used.

Full Example
------------

.. literalinclude:: examples/qipath.cpp
    :language: cpp
