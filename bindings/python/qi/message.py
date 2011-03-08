#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
##

import _qi as qi

def _write_single(message, signature, data):
    if s == "i":
        message.write_int(d)
        return (1, 1)
    elif s == 's':
        message.write_string(d)
        return (1, 1)
    elif s == '[':
        return _write_list(message, signature[i:], data[:di])

def _write_list(message, signature, data):
    length = message.write_int(len(data))
    (i, di) = (1, 1)
    for i in range(length):
        (i, di) += message.read_single(message, signature[i:], data[di:])
    return (1, di)

def _write_map(message, signature, data):
    length = message.write_int(len(data))
    (i, di) = (1, 1)
    for i in range(length):
        (i, di) += _write_single(message, signature[i:], data[di:])
        (i, di) += _write_single(message, signature[i:], data[di:])
    return (1, di)

def _write(message, signature, data):
    di = 0
    length = len(signature)
    dlength = len(data)
    while i < length:
        i  += 1
        di += 1
        (i, di) = _write_single(message, signature[i:], data[di:])
    if di != dlength:
        print "Fail in _write(%s): %d != %d" % (signature, di, dlenght)

def _read(message, signature, data):
    pass

class Message:
    def __init__(self):
        self.pmessage = qi.qi_create_message()

    def decode(self, signature):
        return _read(self, signature)

    def fill(self, signature, data):
        _write(self, signature, data)

    def write_bool(self, data)  : _qi.qi_message_write_bool(self.pmessage, data)
    def write_char(self, data)  : _qi.qi_message_write_char(self.pmessage, data)
    def write_int(self, data)   : _qi.qi_message_write_int(self.pmessage, data)
    def write_float(self, data) : _qi.qi_message_write_float(self.pmessage, data)
    def write_double(self, data): _qi.qi_message_write_double(self.pmessage, data)
    def write_string(self, data): _qi.qi_message_write_string(self.pmessage, data)

    def read_bool(self)   : return _qi.qi_message_read_bool(self.pmessage)
    def read_char(self)   : return _qi.qi_message_read_char(self.pmessage)
    def read_int(self)    : return _qi.qi_message_read_int(self.pmessage)
    def read_float(self)  : return _qi.qi_message_read_float(self.pmessage)
    def read_double(self) : return _qi.qi_message_read_double(self.pmessage)
    def read_string(self) : return _qi.qi_message_read_string(self.pmessage)


def message_to_python(message):
    #return a python type
    pass

def python_to_message(data, signature):
    #return a message
    pass
