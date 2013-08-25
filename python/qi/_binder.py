##
## Author(s):
##  - Julien Freche <jfreche@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012, 2013 Aldebaran Robotics
##

""" Those class are meant to be used as a decorator.

    Bind a function with a signature:
    @qi.bind(qi.Dynamic, (qi.Dynamic, qi.Dynamic))
    def foo():
      pass

    do not bind a function:
    @qi.nobind
    def foo():
      pass


.. module:: qi

"""

import types
from _type import AnyArguments

class bind():
  def __init__(self, retType, paramsType, methodName = None):
    """ bind constructor

    .. args::
       sig : Signature to bind to the function.
    """
    #return value
    if isinstance(retType, types.StringType):
      self._retsig = retType
    else:
      self._retsig = retType.signature

    #parameters
    if isinstance(paramsType, types.StringType):
      self._sig = paramsType
    else:
      if isinstance(paramsType, types.TupleType) or isinstance(paramsType, types.ListType):
        self._sig = "(%s)" % "".join([x.signature for x in paramsType])
      elif isinstance(paramsType, AnyArguments) or issubclass(paramsType, AnyArguments):
        self._sig = "m"
      else:
        self._sig = "(%s)" % paramsType.signature

    self._name = methodName

  def __call__(self, f):
    """ Function generator.
        Associate a function with the signature given to the constructor.
        Return the function binded

    .. args::
       f : function to bind.
    """
    f.__qi_name__ = self._name
    f.__qi_signature__ = self._sig
    f.__qi_return_signature__ = self._retsig
    return f

class nobind():
  def __init__(self, _):
    pass

  def __call__(self, f):
    """ Function generator.
        Return the function with tag to prevent binding

    .. args::
       f : function to bind.
    """
    f.__qi_signature__ = "DONOTBIND"
    return f
