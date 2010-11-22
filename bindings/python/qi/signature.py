#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##


Bool   = "b"
Char   = "c"
Int    = "i"
String = "s"
Void   = "v"

def Protobuf(Name):
    """
    >>> Protobuf("toto")
    '@toto@'
    """
    return "@%s@" % (Name)

def Map(Key, Value):
    """
    >>> Map(String, String)
    '[ss]'
    >>> Map(Map(String, String), Int)
    '[[ss]i]'
    """
    return "[%s%s]" % (Key, Value)

def Function(Return=None, *args):
    """
    >>> Function (String, String)
    's:s'
    """
    if not Return:
        Return = ''
    return "%s:%s" % (Return, "".join(args))

def Signature(Name, Return=None, *args):
    """
    >>> Signature("motion", Void, Int, Int)
    'motion::v:ii'
    """
    return "%s::%s" % (Name, Function(Return, *args))

if __name__ == "__main__":
    print "Test signature:"
    import doctest
    doctest.testmod()
