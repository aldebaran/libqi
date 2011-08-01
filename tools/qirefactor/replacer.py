#!/usr/bin/env python
##
## Author(s):
##  - Dimitri MEREJKOWSKY <yannicklm1337@gmail.com>
##  - Cedric GESTES <ctaf42@gmail.com>
##
## Original Implementation provided by Dimitri
##

"""
Quick script to replace stuff in files

"""

import sys
import os
import re
import random
import logging
import fnmatch

from .context import Context, PyContext

COLORS = {
    "clear"         : "\033[0m"  ,

    "bold"          : "\033[1m"  ,
    "underline"     : "\033[4m"  ,

    "red"           : "\033[0;31m" ,
    "light-red"     : "\033[1;31m" ,

    "green"         : "\033[0;32m" ,
    "light-green"   : "\033[1;32m" ,

    "blue"          : "\033[0;34m" ,
    "light-blue"    : "\033[1;34m" ,

    "magenta"       : "\033[0;36m" ,
    "light-magenta" : "\033[1;36m" ,
}

NO_COLORS = {
    "clear"         : "\033[0m"  ,

    "bold"          : "\033[1m"  ,
    "underline"     : "\033[4m"  ,

    "red"           : "\033[0;31m" ,
    "light-red"     : "\033[1;31m" ,

    "green"         : "\033[0;32m" ,
    "light-green"   : "\033[1;32m" ,

    "blue"          : "\033[0;34m" ,
    "light-blue"    : "\033[1;34m" ,

    "magenta"       : "\033[0;36m" ,
    "light-magenta" : "\033[1;36m" ,
}

COLORS_REPLACE = {
    "context"    : COLORS["clear"] + COLORS["light-magenta"],
    "line1"      : COLORS["clear"],
    "line2"      : COLORS["clear"],
    "line1start" : COLORS["clear"] + COLORS["light-red"],
    "line2start" : COLORS["clear"] + COLORS["light-green"],
    "word1"      : COLORS["clear"] + COLORS["underline"] + COLORS["light-red"],
    "word2"      : COLORS["clear"] + COLORS["underline"] + COLORS["light-green"],
}



def grep_file(fname, regexp, context = True, quiet = False):
    with open(fpath, "r") as in_fd:
        grep_lines(in_fd.readlines(), regexp, context, quiet)

def grep_line(line, regexp, colormap, context = True):
    line = line.rstrip()
    match    = re.search(regexp, line)
    line_color   = line[0:match.start()] + colormap["word1"]
    line_color  += line[match.start():match.end()]
    line_color  += colormap["line2"] + line[match.end():]
    print "%s%s: %s%s%s" % (colormap["line2start"], ln, colormap["line2"], line_color, colormap["clear"])

def grep_lines(lines, regexp, colormap = None, context = True, quiet = False):
    """ display math """
    if context:
        ctx = PyContext(fpath)
    else:
        ctx = Context(fpath)
    #TODO: handle None colormap
    display_header = True
    for out_line, ln in zip(in_lines, range(len(in_lines))):
        ctx.search(out_line)
        if re.search(regexp, out_line):
            if display_header and not quiet:
                print
                print colormap["bold"] + colormap["light-blue"] + "file: " + os.path.relpath(fpath) + colormap["clear"]
                display_header = False
            ctx.display()
            out_line = out_line.rstrip()
            match    = re.search(regexp, out_line)
            out_line_color  = out_line[0:match.start()] + colormap["word1"]
            out_line_color  = out_line_color + out_line[match.start():match.end()]
            out_line_color  = out_line_color + colormap["line2"] + out_line[match.end():]
            print "%s%s: %s%s%s" % (colormap["line2start"], ln, colormap["line2"], out_line_color, colormap["clear"])

        # for k in COLORS.iterkeys():
        #     COLORS[k] = ""
        # for k in COLORS_REPLACE.iterkeys():
        #     COLORS_REPLACE[k] = ""


def sed_file(fpath, regexp, replacement,
             colormap = None,
             quiet = False,
             color = True,
             backup = True):
    """
    Perfoms re.sub(regexp, repl, line) for each line in fpath
    """
    with in_fd as open(fpath, "r"):
        in_lines = in_fd.readlines()


def sed_lines(lines, regexp, replacement,
              colormap = None,
              dryrun = None,
              quiet = False,
              color = True,
              backup = True):
    out_lines = [re.sub(pattern, replacement, l) for l in lines]
    #quit if not transformation
    if in_lines == out_lines:
        return

    if not quiet:
        print colormap["bold"] + colormap["light-blue"] + "patching: " + os.path.relpath(fpath) + colormap["clear"]
    if dryrun:
        if opts.get("backup"):
            fs.backup_file(fpath)
        with open(fpath, "w") as out_fd:
            out_fd.writelines(out_lines)
    if quiet:
        return

    for (in_line, out_line) in zip(in_lines, out_lines):
        if in_line != out_line:
            print_sed_line(in_line, out_line, regexp, repl, colormap)


def print_sed_line(in_line, out_line, regexp, repl, colormap):
    in_line  = in_line.strip()
    out_line = out_line.strip()
    match    = re.search(regexp, in_line)
    in_line_color  = in_line[0:match.start()] + colormap["word1"]
    in_line_color  = in_line_color + in_line[match.start():match.end()]
    in_line_color  = in_line_color + colormap["line1"] + in_line[match.end():]
    out_line_color = re.sub(regexp, colormap["word2"] + repl + colormap["line2"], in_line)
    print "%s--%s %s%s" % (colormap["line1start"], colormap["line1"], in_line_color, colormap["clear"])
    print "%s++%s %s%s" % (colormap["line2start"], colormap["line2"], out_line_color, colormap["clear"])
    print



if __name__ == "__main__":
    main()


