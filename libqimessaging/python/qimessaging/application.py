#!/usr/bin/env python
##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import _qi

class Application
    def __init__(self):
        self.app = _qi.qi_application_create()

    def run(self):
        _qi.qi_application_run(self.app)

    def __del__(self):
        _qi.qi_application_destroy(self.app)
