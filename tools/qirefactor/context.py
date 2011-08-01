#!/usr/bin/env python
##
## context.py
## Login : <ctaf@speedcore>
## Started on  Mon Aug  1 00:46:54 2011
## $Id$
##
## Author(s):
##  -  <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2011
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

class Context:
    """ regexp context """
    def __init__(self, filename):
        self.filename = filename

    #VIRTUAL
    def search(self, line):
        """ search for a context line """
        pass

    #VIRTUAL
    def display(self):
        """ display a context if needed """
        pass

class PyContext(Context):
    def __init__(self, filename):
        Context.__init__(self, filename)
        self.regexp     = re.compile("[ \t]*def[ \t].*\(.*\)[ \t]*:")
        self.match      = None
        self.displayed  = False

    def search(self, line):
        """ search for a function or class name """
        if self.regexp.search(line):
            self.match     = line
            self.displayed = False

    def display(self):
        """ display the current function/class name """
        if self.displayed:
            return
        if not self.match:
            return
        print "%sIn: %s%s" % (COLORS_REPLACE["pyctx"], self.match.strip(), COLORS["clear"])
        self.displayed = True
