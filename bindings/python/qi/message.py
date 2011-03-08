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




def single_value_to_python(sig, message):
    """ convert the first value of a message based on the first type in the sig
    """
    print "single value: %s" % (sig[0])
    if sig[0] == 'b':
        return True if message.read_bool() else False
    elif sig[0] == 'c':
        return message.read_char()
    elif sig[0] == 'i':
        return message.read_int()
    elif sig[0] == 'f':
        return message.read_float()
    elif sig[0] == 'd':
        return message.read_double()
    elif sig[0] == 's':
        print "aie aie aie"
        return message.read_string()
    elif sig[0] == '[':
        print "oups"
        ret = list()
        subsig = qi.signature.split(sig[1:-1])
        if len(subsig) != 1:
            raise qi.signature.BadSignatureException("what?")
        c = message.read_int()
        for i in range(c):
            ret.append(single_value_to_python(subsig[0], message))
        return ret
    elif sig[0] == '{':
        ret = dict()
        subsig = qi.signature.split(sig[1:-1])
        if len(subsig) != 2:
            raise qi.signature.BadSignatureException("what?")
        c = message.read_int()
        for i in range(c):
            k = single_value_to_python(subsig[0], message)
            v = single_value_to_python(subsig[1], message)
            ret[k] = v
        return ret
    raise qi.signature.BadSignatureException("what?")

def message_to_python(sig, message):
    """ convert a message to a native python type.
        result could be :
        - None
        - a single value
        - a list of values

        a value could be a POD, a list, a map
    """
    sigsplit = qi.signature.split(sig)

    if len(sigsplit) == 0:
        return None
    elif len(sigsplit) == 1:
        return single_value_to_python(sigsplit[0], message)
    return [ single_value_to_python(s, message) for s in sigsplit ]

def single_value_to_message(sig, data, message):
    """ write a python type to a message
    """
    if sig[0] == 'b':
        message.write_bool(data)
        return
    elif sig[0] == 'c':
        message.write_char(data)
        return
    elif sig[0] == 'i':
        message.write_int(int(data))
        return
    elif sig[0] == 'f':
        message.write_float(float(data))
        return
    elif sig[0] == 'd':
        message.write_double(float(data))
        return
    elif sig[0] == 's':
        print "towrite:", data
        message.write_string(str(data))
        return
    elif sig[0] == '[':
        listlen = len(data)
        subsig = qi.signature.split(sig[1:-1])
        message.write_int(listlen)
        assert(len(subsig) == 1)
        for i in range(listlen):
            single_value_to_message(subsig[0], data[i], message)
        return
    elif sig[0] == '{':
        dictlen = len(data)
        subsig = qi.signature.split(sig[1:-1])
        assert(len(subsig) == 2)
        message.write_int(dictlen)
        for k,v in data.iteritems():
            single_value_to_message(subsig[0], k, message)
            single_value_to_message(subsig[1], v, message)
        return
    raise qi.signature.BadSignatureException("what?")

def python_to_message(signature, *args):
    """ write a list of python type to a message
    """
    sigsplit = qi.signature.split(signature)

    if len(sigsplit) != len(args):
        raise qi.signature.BadSignatureException()

    message = qi.Message()
    #message.write_string(signature)

    for s,a in zip(sigsplit, args):
        print "converting %s: %s" % (s, a)
        single_value_to_message(s, a, message)
    return message
