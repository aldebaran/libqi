#!/usr/bin/env python
##
## options.py
## Login : <ctaf@speedcore>
## Started on  Mon Aug  1 00:48:04 2011
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

def add_options_replacer(option_parser)
    """ add replacer options """
    option_parser.add_option("--no-skip-hidden",
        action = "store_false", dest = "no_hidden",
        help = "Do not skip hidden files. Use this if you know what you are doing...")
    option_parser.add_option("--file-filter", dest = "file_filter", action = "append",
                             help = "File filter to apply (multiple filters can be specified)")
    option_parser.add_option("--no-filter", action = "store_true", dest = "no_filter",
                             help = "Do not skip files that match the filter")
    option_parser.add_option("-d", "--debug",
        action = "store_true", dest = "debug",
        help = "Enable debug output")
    option_parser.add_option("--backup",
        action = "store_true", dest = "backup",
        help = "Create a backup for each file. This is the default")
    option_parser.add_option("--no-backup",
        action = "store_false", dest = "backup",
        help = "Do not create backups")
    option_parser.add_option("--go",
        action = "store_true", dest = "go",
        help = "Perform changes rather than just printing then")
    option_parser.add_option("--find",
        action = "store_true", dest = "find",
        help = "Only search for match")
    option_parser.add_option("--dry-run", "-n",
        action = "store_false", dest = "go",
        help = "Do not change anything. This is the default")
    option_parser.add_option("--color",
        action = "store_false", dest = "color",
        help = "Colorize output. This is the default")
    option_parser.add_option("--no-color",
        action = "store_false", dest = "color",
        help = "Do not colorize output")
    option_parser.add_option("--quiet", "-q",
        action = "store_true", dest = "quiet",
        help = "Do not produce any output")
    option_parser.add_option("--no-py-ctx",
        action = "store_false", dest = "pyctx",
        help = "Do not use the python context")

    option_parser.set_defaults(
        no_hidden = True,
        no_filter = False,
        backup    = True,
        go        = False,
        color     = True,
        debug     = False,
        quiet     = False,
        pyctx     = True)


def replacer_handle_options(opts):
    if not opts.get("color") or not sys.stdout.isatty():
        for k in COLORS.iterkeys():
            COLORS[k] = ""
        for k in COLORS_REPLACE.iterkeys():
            COLORS_REPLACE[k] = ""

    if opts.get("debug"):
        logging.basicConfig(level=logging.DEBUG)
