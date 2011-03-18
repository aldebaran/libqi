#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
##

import _qi
import qi.signature

class Message:
    def __init__(self):
        self.pmessage = _qi.qi_message_create()

    def write_bool(self  , data): _qi.qi_message_write_bool(self.pmessage, data)
    def write_char(self  , data): _qi.qi_message_write_char(self.pmessage, data)
    def write_int(self   , data): _qi.qi_message_write_int(self.pmessage, data)
    def write_float(self , data): _qi.qi_message_write_float(self.pmessage, data)
    def write_double(self, data): _qi.qi_message_write_double(self.pmessage, data)
    def write_string(self, data): _qi.qi_message_write_string(self.pmessage, data)

    def read_bool(self)   : return _qi.qi_message_read_bool(self.pmessage)
    def read_char(self)   : return _qi.qi_message_read_char(self.pmessage)
    def read_int(self)    : return _qi.qi_message_read_int(self.pmessage)
    def read_float(self)  : return _qi.qi_message_read_float(self.pmessage)
    def read_double(self) : return _qi.qi_message_read_double(self.pmessage)
    def read_string(self) : return _qi.qi_message_read_string(self.pmessage)


def message_to_python(sig, message):
    """ convert a message to a native python type.
        result could be :
        - None
        - a single value
        - a list of values

        a value could be a POD, a list, a map
    """
    return _qi.qi_message_to_python(sig, message.pmessage)


def python_to_message(sig, message, *args):
    """ write a list of python type to a message
    """
    _qi.qi_python_to_message(sig, message.pmessage, args)
    return message
