#!/usr/bin/env python
##
## replacer.py
## Login : <ctaf@speedcore>
## Started on  Mon Aug  1 00:50:22 2011
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


from optparse import OptionParser
from qirefactor.options import add_options_replacer
from qirefactor. import toto


DIR_BLACKLIST = ( "build*", ".build*", ".dep", # build directories
                  ".git", ".svn" )             # VCS

FILE_BLACKLIST = ( "*.back"  , "*~",          # backup
                   "*.py[co]",                # python
                   "*.[oa]",                  # gcc
                   "*.exe", "*.dll", "*.lib", # win32
                   "*.dylib",                 # mac
                   "*.so" )                   # linux

__usage__ = """
replacer [options]  PATTERN REPL [files]

eg:
  replacer 'toto' 'titi'
  replacer '(.*)toto([0-9]{0,3})' '\\1titi\\2'

Files matching %s are discarded.
""" % (str(FILTER_OUT))


LOGGER = logging.getLogger("replacer")
def find_main(opts, args):
    """ find main """
    if len(args) < 1:
        print "Wrong number of arguments"
        print __usage__
        sys.exit(2)

    pattern = args[0]
    regexp = re.compile(pattern)

    def find_action(f):
        return find_in_file(f, regexp, opts)

    if len(args) > 1:
        files = args[1:]
        for f in files:
            find_action(f)
    else:
        recurse_file(os.getcwd(), find_action, opts)


def replacer_file(pattern, replacement, opts):
def repl_main(opts, args):
    """ replacer main """
    if len(args) < 2:
        print "Wrong number of arguments"
        print __usage__
        sys.exit(2)

    pattern = args[0]
    repl    = args[1]
    regexp  = re.compile(pattern)

    def repl_action(f):
        return replace_in_file(opts, f, regexp, repl)

    if len(args) > 2:
        files = args[2:]
        for f in files:
            repl_action(f)
    else:

        recurse_file(directory, lambda x: replace_in_file(x, regexp, replacement), opts)


def main():
    """
    manages options when called from command line

    """
    option_parser = OptionParser(usage = __usage__)
    add_options_replacer(option_parser)
    #TODO
    (opts_obj, args) = option_parser.parse_args()

    opts = vars(opts_obj)
    color = not sys.stdout.isatty()
    replacer_handle_options(opts)

    if opts.get("find"):
        find_main(opts, args)
    else:
        repl_main(opts, args)
