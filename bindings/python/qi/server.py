#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
##

import _qi

class Server:
    def __init__(self, name, context=None):
        """
        """
        self.pclient = _qi.qi_server_create(name)

    def connect(self, address):
        """ connect to a master """
        return _qi.qi_server_connect(self.pclient, address)

    def advertise_service(self, servicename, func):
        return _qi.qi_server_advertise_python_service(self.pclient, servicename, func)
        pass

    def unadvertise_service(self, servicename):
        pass

