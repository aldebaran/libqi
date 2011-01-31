#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2010 Aldebaran Robotics
##

#this tools, rewrite c/c++ headers

import sys
import re
import os
import random

regex_pragma_once = re.compile("^\s*\#pragma\s+once\s*$")

def read_header(fname):
    """ check for define guard in a c/c++ header """
    headers = list()
    body    = list()
    lines   = []

    with open(fname, "rb") as f:
        lines = f.readlines()

    getheader = True
    for ln in range(len(lines)):
        stripped = lines[ln].strip()
        if stripped != "" and not stripped.startswith("//") and not regex_pragma_once.match(stripped) and not stripped.startswith("/*") and not stripped.startswith("*") and not stripped.startswith("*/"):
            getheader = False
        if regex_pragma_once.match(stripped):
            print "removed pragma"
            continue
        if getheader:
            headers.append(lines[ln])
        else:
            body.append(lines[ln])
    return (headers, body)

def write_header(fname, header, body):
    """ rewrite the header guard lines """
    with open(fname, "wb") as f:
        for l in header:
            f.write(l)
        f.write("#pragma once\n\n")
        for l in body:
            f.write(l)

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
        update-header-pragme.py qi
    """
    dest = sys.argv[1]
    flist = ls_r(dest, ".*\.(h|hpp|hxx)$")
    flist = [ os.path.join(dest, x) for x in flist ]
    for f in flist:
        print "checking file:", f
        (header, body) = read_header(f)
        write_header(f, header, body)
