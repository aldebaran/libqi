##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import _qi

class CallError(Exception):
    def __init__(self, value):
       self.value = value
    def __str__(self):
        return repr(self.value)

class Object:

    def __init__(self, qi_object = None):
        if qi_object is None:
            self.obj = _qi.qi_object_create("obj")
        else:
            self.obj = qi_object

    def register_method(self, signature, func, data):
        _qi.qi_object_register_method(self.obj, signature, func, data)

    def call(self, name, *args):
        return _qi.qi_generic_call(self.obj, name, args)

    def __del__(self):
        _qi.qi_object_destroy(self.obj)
