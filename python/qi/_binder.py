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
import inspect
from _type import AnyArguments

class bind():
  def __init__(self, retType, paramsType = None, methodName = None):
    """ bind constructor

    .. args::
       sig : Signature to bind to the function.
    """
    #return value
    if retType is None:
      self._retsig = None
    else:
      self._retsig = str(retType)

    #parameters
    if paramsType is None:
      self._sig = None
    elif isinstance(paramsType, types.TupleType) or isinstance(paramsType, types.ListType):
      self._sig = "(%s)" % "".join([str(x) for x in paramsType])
    elif isinstance(paramsType, AnyArguments) or (inspect.isclass(paramsType) and issubclass(paramsType, AnyArguments)):
      self._sig = "m"
    else:
      raise Exception("Invalid types for parameters")
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

class singleThreaded():
    def __init(self, _):
        pass

    def __call__(self, f):
        """ Function generator.
            Return the function with tag to prevent binding

        .. args::
            f : function to bind.
        """
        f.__qi_threading__ = "single"
        return f

class multiThreaded():
    def __init(self, _):
        pass

    def __call__(self, f):
        """ Function generator.
            Return the function with tag to prevent binding

        .. args::
            f : function to bind.
        """
        f.__qi_threading__ = "multi"
        return f
