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

#this tools, rewrite the define guard of c/c++ headers.
#
# for example.h :
# ifndef BADPROTECT
# define BADPROTECT
# ...
# endif
#
# ===>
# ifndef _EXAMPLE_H_
# define _EXAMPLE_H_
# ...
# endif  // _EXAMPLE_H_
#

import sys
import re
import os

regex_ifndef = re.compile("^\s*\#\s*ifndef\s+(\S*)\s*$")
regex_define = re.compile("^\s*\#\s*define\s+(\S*)\s*$")
regex_endif  = re.compile("^\s*\#\s*endif")


def read_header_guard(fname):
    """ check for define guard in a c/c++ header """
    lines = []
    with open(fname, "r") as f:
        lines = f.readlines()

    ln_ifdef  = -1
    ln_define = -1
    ln_endif  = -1
    guard = None
    for ln in range(len(lines)):
        line = lines[ln].strip()
        match_ndef = regex_ifndef.match(line)
        if match_ndef and not guard:
            guard = match_ndef.group(1)
            ln_ifdef = ln
            #print "match   :", guard
            continue

        match_def = regex_define.match(line)
        if match_def and ln_define == -1:
            if match_def.group(1) == guard:
                ln_define = ln
                #print "match   :", match_def.group(1)

        match_end = regex_endif.match(line)
        if match_end:
            ln_endif = ln
            #print "match   : end"
    return (guard, ln_ifdef, ln_define, ln_endif)

def write_header_guard(fname, guard, pos):
    """ rewrite the header guard lines """
    lines = []
    with open(fname, "r") as f:
        lines = f.readlines()
    with open(fname, "w") as f:
        for i in range(len(lines)):
            line = lines[i]
            if i == pos[0]:
                f.write("#ifndef %s\n" % (guard))
            elif i == pos[1]:
                f.write("#define %s\n" % (guard))
            elif i == pos[2]:
                f.write("#endif  // %s\n" % (guard))
            else:
                f.write(line)
    pass

def generate_name(fname):
    """ generate a proper guard name """
    ret = fname.upper()
    ret = ret.replace("\\", "_")
    ret = ret.replace("/", "_")
    ret = ret.replace(".", "_")
    ret = ret.replace("-", "_")
    return "_%s_" % (ret)
    return ret

def ls_r(directory, pattern):
    """Returns a sorted list of all the files present in a diretory,
    relative to this directory.
    """
    myreg = re.compile(pattern)
    res = list()
    for root, dirs, files in os.walk(directory):
        new_root = os.path.relpath(root, directory)
        if new_root == "." and not files:
            continue
        if new_root == "." and files:
            res.extend(files)
            continue
        if not files and not dirs:
            res.append(new_root + os.path.sep)
            continue
        for f in files:
            res.append(os.path.join(new_root, f))
    return [ x for x in res if myreg.match(x) ]

if __name__ == "__main__":
    """ for example try :
        checkheader.py qi
    """
    dest = sys.argv[1]
    flist = ls_r(dest, ".*\.h(pp|xx)$")
    flist = [ os.path.join(dest, x) for x in flist ]
    for f in flist:
        print "checking file:", f
        (guard, l1, l2, l3) = read_header_guard(f)
        guard = generate_name(f)
        print "new name: ", guard
        write_header_guard(f, guard, (l1, l2, l3))
