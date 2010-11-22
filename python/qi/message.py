#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
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
    while for i < length:
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

    def write_int(self, data):
        qi.qi_message_write_int(self.pmessage, data)

    def write_string(self, data):
        qi.qi_message_write_string(self.pmessage, data)

    def read_int(self):
        return qi.qi_message_read_int(self.pmessage)

    def read_string(self):
        return qi.qi_message_read_string(self.pmessage)
