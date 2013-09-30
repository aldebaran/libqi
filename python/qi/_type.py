##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 - 2013 Aldebaran Robotics
##

import _qi
import types

#allow print str(Void)
class _MetaSignature(type):
    def __str__(self):
        return self.signature

    #support comparing class and instance (Int8 == Int8())
    def __eq__(self, other):
        if isinstance(other, types.StringType):
            return other == self.signature
        return other.signature == self.signature
    def __ne__(self, other):
        if isinstance(other, types.StringType):
            return other != self.signature
        return other.signature != self.signature

#allow print str(Void())
class _Signature(object):
    __metaclass__ = _MetaSignature
    def __str__(self):
        return self.signature

    def __eq__(self, other):
        if isinstance(other, types.StringType):
            return other == self.signature()
        return other.signature == self.signature
    def __ne__(self, other):
        return not self.__eq__(other)

class Void(_Signature):
    signature = 'v'

class Bool(_Signature):
    signature = 'b'

class Int8(_Signature):
    signature = 'c'

class UInt8(_Signature):
    signature = 'C'

class Int16(_Signature):
    signature = 'w'

class UInt16(_Signature):
    signature = 'W'

class Int32(_Signature):
    signature = 'i'

class UInt32(_Signature):
    signature = 'I'

class Int64(_Signature):
    signature = 'l'

class UInt64(_Signature):
    signature = 'L'

class Float(_Signature):
    signature = 'f'

class Double(_Signature):
    signature = 'd'

class String(_Signature):
    __metaclass__ = _MetaSignature
    signature = "s"

class List(_Signature):
    def __init__(self, elementType):
        self.signature = "[%s]" % elementType.signature

class Map(_Signature):
    def __init__(self, keyType, elementType):
        self.signature = "{%s%s}" % (keyType.signature, elementType.signature)

class Struct(_Signature):
    def __init__(self, fields):
        self.signature = "(%s)" % fields.join("")

class Object(_Signature):
    __metaclass__ = _MetaSignature
    signature = 'o'

class Dynamic(_Signature):
    __metaclass__ = _MetaSignature
    signature = 'm'

class Buffer(_Signature):
    __metaclass__ = _MetaSignature
    signature = 'r'

#yes this look similar to Dynamic but it's not.
#eg: qi.bind(Void, (Dynamic, Dynamic))  this mean a tuple of two dynamic.
#eg: qi.bind(Void, AnyArguments)        this is not a tuple. (m not in tuple, mean anythings)
#eg: qi.bind(Void, Dynamic)             this is a function with one argument
class AnyArguments(_Signature):
    __metaclass__ = _MetaSignature
    signature = 'm'


#return the qi.type of the parameter
def typeof(a):
    if isinstance(a, _qi.Object):
        return Object
    raise NotImplementedError("typeOf is only implemented for Object right now")

#cant be called isinstance or typeof will run into infinite loop
#see qi.__init__ for the renaming
def _isinstance(a, type):
    if type != Object:
      raise NotImplementedError("isinstance is only implemented for Object right now")
    try:
      return typeof(a) == type
    except NotImplementedError:
      return False
