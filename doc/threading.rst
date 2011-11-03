Threading
=========

By default all application are written using asynchronous calls.

A box with an input that dont return within 10msec is considered to be failing. Thread should be created to do real computation.

each behavior is single threaded. Thread could be added when required. We follow a reactor pattern.
We register handler for signals, and emit signals. The code should not be blocking.


Objects
=======

