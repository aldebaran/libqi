#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

import _qi

def _explode(sig):
    if sig.startswith("::"):
        sig = sig[2:]
    return sig.split(":", 1)

class Client:
    def __init__(self, name, address):
        """
        """
        self.pclient = _qi.qi_create_client(name, address)

    def call(self, method, signature, *args):
        """
        """
        (retsig, callsig) = explode(signature)
        message = Message()
        message.fill(callsig, args)
        mereturn.pmessage = _qi.qi_call_client(self.pclient, method, message.pmessage)
        return mereturn.decode(retsig)
