##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import _qi

class Future:
    def __init__(self, future):
        self.fut = future

    def wait(self):
        _qi.qi_future_wait(self.fut)

    def is_error(self):
        return _qi.qi_future_is_error(self.fut)

    def is_ready(self):
        return _qi.qi_future_is_ready(self.fut)

    def get_value(self):
        return None

    def get_error(self):
        return _qi.qi_future_get_error(self.fut)

    def __del__(self):
        _qi.qi_future_destroy(self.fut)
