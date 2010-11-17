#!/usr/bin/env python
##
## signature.py
## Login : <ctaf42@cgestes-de2>
## Started on  Wed Nov 17 13:45:26 2010 Cedric GESTES
## $Id$
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Cedric GESTES
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##


Bool   = "b"
Char   = "c"
Int    = "i"
String = "s"
Void   = "v"

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
