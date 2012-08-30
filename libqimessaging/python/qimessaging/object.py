#!/usr/bin/env python
##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import _qi
import qi

class Object:
    def __init__(self):
        self.obj = _qi.qi_object_create()

    def registerMethod(self, signature, func, data):
        _qi.qi_object_register_method(self.obj, signature, func, data)

    def call(self, signature, message):
        _qi.qi_object_call(self.obj, signature, message)

    def __del__(self):
        _qi.qi_object_destroy(self)
