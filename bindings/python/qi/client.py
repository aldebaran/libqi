#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
##

import _qi
import qi

class Client:
    def __init__(self, name, context=None):
        """
        """
        self.pclient = _qi.qi_client_create(name)

    def connect(self, address):
        """ connect to a master """
        return _qi.qi_client_connect(self.pclient, address)

    def raw_call(self, method, signature, message):
        ret = qi.Message()
        if not isinstance(message, qi.Message):
            raise Exception("message is not of type qi.Message")
        _qi.qi_client_call(self.pclient, message.pmessage, ret.pmessage)
        return ret

    def call(self, methodSignature, *args):
        """ """
        return _qi.qi_client_python_call(self.pclient, methodSignature, args)

    def locate_service(self, methodSignature):
        return  _qi.qi_master_locate_service(self.pclient, methodSignature)
