.. _api-py-translator:

qi.Translator API
*****************

Introduction
============

Translator is the way to internationalize strings.
It works with `qilinguist`.

Reference
=========

.. autoclass:: qi.Translator
   :members:

Examples
========

Simple example:

.. code-block:: python

   import qi

   tr = qi.Translator("my_app")
   tr.setDefaultDomain("my_domain")
   tr.setCurrentLocale("fr_FR")

   translated_mess = tr.translate("Hi, I am %s!")

   print translated_mess % ("Nicolas")
   # > "Salut, je m'apelle Nicolas !"

   # You can change domain only for a translation
   print tr.translate("Hi!", "formal")
   # > "Bonjour !" (means "Good morning!")

   print tr.translate("Hi!", "casual")
   # > "Salut !" (means "Hi bro!")

   # You can also change language
   print tr.translate("Hi!", "formal", "de_DE")
   # > "Guten Tag !"

   print tr.translate("Hi!", "casual", "de_DE")
   # > "Hallo !"
