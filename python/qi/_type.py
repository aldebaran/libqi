##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 - 2013 Aldebaran Robotics
##

class Void(object):
    signature = 'v'

class Int(object):
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

class Float(object):
    #default sig of type 'd', use ctor to refine
    signature = 'd'
    def __init__(self, size=8):
        if size == 4:
            self.signature = 'f'
        elif size == 8:
            self.signature = 'd'
        else:
            raise Exception("Invalid size for float: %d" % (size))

class String(object):
    signature = "s"

class List(object):
    def __init__(self, elementType):
        self.signature = "[%s]" % elementType.signature

class Map(object):
    def __init__(self, keyType, elementType):
        self.signature = "{%s%s}" % (keyType,signature, elementType.signature)

class Struct(object):
    def __init__(self, fields):
        self.signature = "(%s)" % fields.join("")

class Object(object):
    signature = 'o'

class Dynamic(object):
    signature = 'm'

class Buffer(object):
    signature = 'r'
