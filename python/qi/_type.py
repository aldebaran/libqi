##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 - 2013 Aldebaran Robotics
##

#allow print str(Void)
class _MetaSignature(type):
    def __str__(self):
        return self.signature

#allow print str(Void())
class _Signature(object):
    def __str__(self):
        return self.signature

class Void(_Signature):
    __metaclass__ = _MetaSignature
    signature = 'v'


class Int(_Signature):
    __metaclass__ = _MetaSignature
    #default sig of type 'l', use ctor to refine
    signature = 'l'

    def __init__(self, size=8, signed=True):
        #unsigned: UPPER CASE
        ar = { -8 : 'L',
               -4 : 'I',
               -2 : 'W',
               -1 : 'C',
                0 : 'b',
                1 : 'c',
                2 : 'w',
                4 : 'i',
                8 : 'l', }
        if signed:
            self.signature = ar[size]
        else:
            self.signature = ar[-size]


class Float(_Signature):
    __metaclass__ = _MetaSignature
    #default sig of type 'd', use ctor to refine
    signature = 'd'
    def __init__(self, size=8):
        if size == 4:
            self.signature = 'f'
        elif size == 8:
            self.signature = 'd'
        else:
            raise Exception("Invalid size for float: %d" % (size))

class String(_Signature):
    __metaclass__ = _MetaSignature
    signature = "s"

class List(_Signature):
    __metaclass__ = _MetaSignature
    def __init__(self, elementType):
        self.signature = "[%s]" % elementType.signature

class Map(_Signature):
    __metaclass__ = _MetaSignature
    def __init__(self, keyType, elementType):
        self.signature = "{%s%s}" % (keyType,signature, elementType.signature)

class Struct(_Signature):
    __metaclass__ = _MetaSignature
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
