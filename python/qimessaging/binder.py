##
## Author(s):
##  - Julien Freche <jfreche@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

""" This Class is meant to be used as a decorator.
    Bind a function with a signature.

.. module:: qimessaging

"""

import inspect

class bind():
  def __init__(self, sig):
    """ bind constructor

    .. args::
       sig : Signature to bind to the function.
    """
    self.sig = sig

  def __call__(self, f):
    """ Function generator.
        Associate a function with the signature given to the constructor.
        Return the function binded

    .. args::
       f : function to bind.
    """
    f.__qi_signature__ = f.__name__ + "::" + self.sig
    return f

class nobind():
  def __init__(self, f):
    """ nobind constructor

    """

  def __call__(self, f):
    """ Function generator.
        Return the function with tag to prevent binding

    .. args::
       f : function to bind.
    """
    f.__qi_not_bind__ = ""
    return f

def generateDefaultSignature(f):
  try:
    argspec = inspect.getargspec(f)
  except TypeError as e:
    return None
  argNb = len(argspec[0])
  if (argspec[1] is not None):
    argNb = argNb + 1
  if (argspec[2] is not None):
    argNb = argNb + 1
  return f.__name__ + "::m(" + argNb * "m" + ")"

def getFunctionSignature(f):
  signature = None

  # This function is tagged as private
  if (hasattr(f, "__qi_not_bind__")):
    return

  # Function has a binded signature
  if (hasattr(f, "__qi_signature__")):
    signature = getattr(f, "__qi_signature__")
  # Bind the function with dynamic parameters
  else:
    signature = generateDefaultSignature(f)

  return signature

def buildFunctionListFromObject(obj):
  # Construct a list of tuple with function and signature associated
  functionsList = []
  attrs = dir(obj)

  for attr in attrs:
    if (attr.startswith("_")):
      continue
    fun = getattr(obj, attr)
    signature = getFunctionSignature(fun)
    if (signature is not None):
      functionsList.append((fun, signature))

  return functionsList

