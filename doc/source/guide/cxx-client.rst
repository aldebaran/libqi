.. _guide-cxx-client:

.. cpp:namespace:: qi

.. cpp:auto_template:: True

.. default-role:: cpp:guess

How to write a qimessaging client in C++
========================================

Introduction
-------------

This guide will teach you how to write a qimessaging client in C++, to
interact with the various services offered by the robot.

Prerequisites
-------------

- An installed NAOqi SDK for your operating system.
- A C++ project setup in your favorite C++ development environment, ready
  to use the headers and libraries provided by the NAOqi SDK.

Creating a session
------------------

The first step to interact with qimessaging services is to connect a
:cpp:class:`qi::Session` to the :ref:`Service Directory<api-ServiceDirectory>`
of the robot. The Service Directory address is represented by a standard URL.

One simple way to achive this is to use the helper class _`ApplicationSession`,
which will fetch the URL from the command line (using the *--qi-url* option),
or use a default value of *localhost*:

.. code-block:: cpp

  #include <iostream>
  #include <qi/applicationsession.hpp>

  int main(int argc, char** argv)
  {
    qi::ApplicationSession app(argc, argv);
    qi::SessionPtr session = app.session();
    std::cerr << "Ready to roll!" << std::endl;
  }

Using a service
---------------

You can use the `service()` method to get access to a service (local or
remote). This method will return a `qi::AnyObject`.

.. code-block:: cpp

  #include <qi/applicationsession.hpp>

  int main(int argc, char** argv)
  {
    qi::ApplicationSession app(argc, argv);
    qi::SessionPtr session = app.session();
    qi::AnyObject tts = session.service("ALTextToSpeech");
    tts.call<void>("say", "Hello world!");
  }

The `tts` object works as any other `qi::AnyObject`, please refer to its
documentation for more detail.
