.. _guide-cxx-service:

How to write a qimessaging service in C++
=========================================

This document will teach you how to write your very own qimessaging service,
so that your new shiny object can be used by other qimessaging services and
client, locally or remotely, and from many different programming languages.

The first step is to check if a standard interface that matches what your
object does already exists. For instance if you are implementing or wrapping
a new Text-To-Speech engine, you should reuse the existing *ALTextToSpeech*
interface. If no corresponding interface exists, you'll have to write your own.

Write your interface
--------------------

The interface is a very important part of your service. It defines the list of
methods, signals and properties that are made available through qimessaging.

Create a new header file, and write a C++ pure interface (meaning all methods
should be pure virtuals), with the addition that you can use public _`qi::Signal`
and _`qi::Property`. Here is an example exhibiting most of the features::

  // Helper struct
  struct Mosquito
  {
    double yaw,theta,distance;
  };
  class Cat
  {
    public:
      virtual ~Cat() {}
      virtual void meow(int volume) = 0;
      virtual bool setTarget(const Mosquito& m) = 0;

      qi::Property<float> hunger;
      qi::Property<float> boredom;
      qi::Property<float> cuteness;
      qi::Signal<Mosquito> onTargetDetected;
  };


  
Implementing an existing interface
----------------------------------

Generate code

Registering service

Deploy(?)

