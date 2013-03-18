#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics

import qi

class ToTo:
  pass


if __name__ == "__main__":
  plouf = ToTo()
  prom = qi.Promise()
  fut = prom.future()
  prom.set_value(plouf)
  print fut.value()
