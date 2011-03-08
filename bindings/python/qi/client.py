#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
##

import _qi

def _explode(sig):
    if sig.startswith("::"):
        sig = sig[2:]
    return sig.split(":", 1)

class Client:
    def __init__(self, name, context=None):
        """
        """
        self.pclient = _qi.qi_create_client(name, context)

    def connect(self, address):
        """ connect to a master """
        return _qi.qi_client_connect(self.pclient, address)

    def raw_call(self, method, signature, message):
        ret = qi.Message()
        if not isinstance(message, qi.Message):
            raise Exception("message is not of type qi.Message")
        _qi.qi_call_client(self.pclient, message.pmessage, ret.pmessage)
        return ret

    def call(self, method, signature, *args):
        """ """
        (retsig, callsig) = explode(signature)
        message = Message()
        message.fill(callsig, args)
        mereturn.pmessage = _qi.qi_call_client(self.pclient, method, message.pmessage)
        return mereturn.decode(retsig)

    def locate_service(self, methodSignature):
        return  _qi.qi_master_locate_service(self.pclient, methodSignature)
