#!/usr/bin/env python
##
## refactor.py
## Login : <ctaf@speedcore>
## Started on  Mon Aug  1 00:46:13 2011
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

from qirefactor.replacer import grep_file, sed_file
from qirefactor.browse   import foreach_file
from qirefactor.browse   import DEV_DIR_BLACKLIST, DEV_FILE_BLACKLIST

class ReplacerOptions:
    __options__ = ( "dryrun", "quiet", "color", "backup")
    def __init__():
        pass

def replacer(folder, regexp, replacement, options):
    pass

def replacer(folder, regexp, replacement,
             dryrun = None,
             quiet = False,
             color = True,
             backup = True):
    file_action = lambda x: replace_in_file(x, regexp, replacement,
                                            dryrun=dryrun,
                                            quiet=quiet,
                                            color=color,
                                            backup=backup)
    (dc, fc) = foreach_file(folder, file_action=file_action,
                            dir_blacklist = DEV_DIR_BLACKLIST,
                            file_blacklist = FILE_DIR_BLACKLIST)
    pass



def replace_include(old_include_name, new_include_name, public_header=True):
    pattern = '(\s*#\s*include\s+)["<]\s*' + old_include_name + '[">]'
    if public_header:
        replace = '\1<' + new_include_name + ">"
    else:
        replace = '\1"' + new_include_name + '"'

def replace_var(old_name, new_name):
    pass

def replace_ptr(name):
    ( '%s::Ptr\(\)', "boost::shared_ptr<%s>" )
    ( "%s::Ptr", "boost::shared_ptr<%s>" )
    replace = "

    replac
replacer $@ "(\s*#\s*include\s+)[\"<]\s*alcore/alptr.h[\">]" "\1<boost/shared_ptr.hpp>"
replacer $@ "AL::ALPtr" "boost::shared_ptr"
replacer $@ "ALPtr"     "boost::shared_ptr"
replacer $@ "AL::ALWeakPtr" "boost::weak_ptr"
replacer $@ "ALWeakPtr"     "boost::weak_ptr"
