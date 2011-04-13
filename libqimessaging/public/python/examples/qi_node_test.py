#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Cedric GESTES
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##

import qi

myqiserverinstance = qi.server(24242)

#Advanced binding:
@qi.bind(myqiserverinstance, "/motion/walk", qi.Function(qi.String, qi.Int, qi.Int))
def motion_walk_binded(myint, myint):
    return "mystring"


#poor binding:
def motion_walk2_binded(myint, myint):
    return "mystring"
qi.server.register("/motion/walk2::s:ii", motion_walk2_binded);


myqiclientinstance = qi.client(24242)
mystring = myqiclientinstance.call(qi.Signature("/motion/walk2", qi.String, qi.Int, qi.Int), 42, 42)

mywalkfunction = myqiclientinstance.getfunction("/motion/walk", qi.String, qi.Int, qi.Int)
mywalkfunction(42, 42)


mystring = myqiclientinstance.call("/motion/walk2::s::ii", 42, 42)
