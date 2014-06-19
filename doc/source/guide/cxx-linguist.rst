.. _guide-cxx-linguist:

.. role:: cpp(code)
   :language: cpp

.. role:: bash(code)
   :language: bash

.. role:: cmake(code)
   :language: cmake

.. role:: xml(code)
   :language: xml

Internationalization Guide
**************************

Overview
========

This guide will explain how to create internationalization support (ie: dictionaries files) and to use it to translate an application.

This will be done in two steps:
    - Using qilinguist, a qibuild tool, to create and generate a dictionary file.
    - Setting up the C++ application to find and translate strings.

qiLinguist How to use qilinguist
================================

Configuration of translation files
++++++++++++++++++++++++++++++++++

First of all you need to create a ``po`` directory into your root application folder and POTFILES.in in it.

::

  helloapp
    |_ po
      |_ POTFILES.in
    |_ src
      |_ main.cpp
      |_ hello.cpp
    |_ headers
      |_ hello.h

Into the POTFILES.in you MUST add the relative path of all files you want to translate. For example, if you only have translation in hello.cpp, the POTFILES.in looks like:

.. code-block:: bash

  $ cat po/POTFILES.in
  src/hello.cpp

Add qilinguist configurations
+++++++++++++++++++++++++++++

You need to add a ``translate`` node into the qiproject.xml file.
This configuration is used by ``qilinguist`` to generate translation files
and installation rules.

The qiproject.xml will look like:

.. code-block:: xml

  <project name="helloApp">
    <translate name="helloApp" domain="hello" linguas="fr_FR en_US" tr="gettext" />
  </project>

Tag definitions:
    - ``name``: The name of the application or library.
    - ``domain``: The name of the generated dictionary.
    - ``linguas``: A list of all locales supported formatted as xx_XX (country and language code)
    - ``tr``: Define if you use gettext or QT internationalization (value can be: "gettext" or "qt").

Dictionaries will be install into:

.. code-block:: bash

  <sdk_path>/share/locale/<name>/<linguas>/LC_MESSAGES/<domain>.mo

How to use qilinguist
+++++++++++++++++++++

Everything is ready to generate and edit dictionaries.

There are two commands to use:
    - qilinguist update: finds the translatable strings in the specified source, header and generates or updates .po (or .ts) translation files.

    .. warning:: The jp_JP.po isn't correct. You need to replace

       .. code-block:: bash

          "Content-Type: text/plain; charset=ASCII\n"

       by

       .. code-block:: bash

          "Content-Type: text/plain; charset=UTF-8\n"

    - qilinguist release: produces MO files out of PO ones or QM files out of TS ones. The format of those files is a binary format that is used by the localized application.

First of all you need to generate or update translatable files by using ``qilinguist update``. This will create files (into ``po`` folder) you can edit using Linguist from the Qt SDK or poedit for instance.

Once you have translated those files, you need to compile them by using ``qilinguist release``.

How to internationalize C++ applications
========================================

Update CMakeLists.txt
+++++++++++++++++++++

You must add the qi_create_trad(projectname "po") into the CMakeLists.txt root file.

.. code-block:: cmake

   cmake_minimum_required(VERSION 2.8)
   project(yourProjectName)
   qi_create_trad(yourProjectName "po")

Set and configure qi::Translator
++++++++++++++++++++++++++++++++

You need to get a default `qi::Translator`, then you MUST set the default domain and current locale.

Steps:
    - Get default Translator (`qi::defaultTranslator`) with ``name`` the name of the application or library.
    - Set default domain (`qi::Translator::setDefaultDomain`) with ``domain`` the name of the generated dictionary.
    - Set current locale (`qi::Translator::setCurrentLocale`) with ``locale`` the name of the default loacle formatted as xx_XX (country and language code)

.. warning:: The ``name`` and ``domain`` must be the same as those define into the qiproject.xml
.. warning:: The ``locale`` must be a locale supported and define into the qiproject.xml, otherwise it will fallback to the default value.

You can add some domains by using `qi::Translator::addDomain`. This will allow `qi::Translator` to look it multiple dictionary.

You can change the locale you want to use by calling `qi::Translator::setCurrentLocale`. This function allows you to set and switch language at runtime.

How to translate sentences
++++++++++++++++++++++++++

You have two choices to translate your sentences:
    - Call `qi::Translator::translate`.
    - Call `qi::tr`.

All sentences will be parsed when calling qilinguist update, then generate translation files (ie: po/ts files). Then, at runtime `qi::Translator::translate` will look for translation regarding initialization done in the last paragraphe.

Example
=======

.. literalinclude:: examples/qitranslate/main.cpp
   :language: cpp
