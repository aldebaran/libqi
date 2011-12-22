#!/usr/bin/python
## Copyright (c) 2011, Aldebaran Robotics
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##     * Redistributions of source code must retain the above copyright
##       notice, this list of conditions and the following disclaimer.
##     * Redistributions in binary form must reproduce the above copyright
##       notice, this list of conditions and the following disclaimer in the
##       documentation and/or other materials provided with the distribution.
##     * Neither the name of the Aldebaran Robotics nor the
##       names of its contributors may be used to endorse or promote products
##       derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
## WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
## DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
## LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
## ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
## SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Fix for include directory changes:
# include <albroker.h> -> <alcommon/albroker.h> and so on

# Usage: refactor-headers /path/to/naoqi/sdk/

import os
import sys
import re
import shutil

# note: not using argparse because some people may
# still be using python2.6
import optparse


HEADER_REGEXP = re.compile(r'^#\s*include\s*[<"](.*?)[">]')

def is_header(filename):
    """ Is this file a header? """
    ext = os.path.splitext(filename)[-1]
    return ext in [".h", ".hh", ".hpp", ".hxx"]

def is_source(filename):
    """ If the file a source? """
    ext = os.path.splitext(filename)[-1]
    ext = ext.lower()
    for exclude in [".git", ".hg", ".svn", ".cvs"]:
        if exclude in filename:
            return False
    return ext in [".h", ".hh", ".hpp", ".hxx",
                   ".c", ".cc", ".cpp", ".cxx"]


def parse_headers(naoqi_sdk):
    """ Build a dictionnary:
    header -> subdirectory

    For instance:

    {
        "alproxy.h"  : "alcommon",
        "almodule.h" : "alcommon",
        "alptr.h"   :  "alcore",
    }
    """
    include_dir = os.path.join(naoqi_sdk, "include")
    alcommon_inc = os.path.join(include_dir, "alcommon")
    if not os.path.exists(include_dir):
        mess  = "{} does not look like a valid NAOqi SDK\n"
        mess += "({} does not exists)"
        raise Exception(mess.format(include_dir, alcommon_inc))

    res = dict()
    subdirs = os.listdir(include_dir)
    subdirs = [x for x in subdirs if x.startswith("al")]
    subdirs = [os.path.join(include_dir, x) for x in subdirs]
    subdirs = [x for x in subdirs if os.path.isdir(x)]

    for subdir in subdirs:
        headers = os.listdir(subdir)
        headers = [x for x in headers if is_header(x)]
        for header in headers:
            if header.endswith("config.h"):
                # There's nothing much we can do here...
                continue
            res[header] = os.path.basename(subdir)

    return res


def fix_headers(filename, headers_dict, backup=True, write=False):
    print "patching", filename
    need_fix = False
    with open(filename, "r") as fp:
        lines = fp.readlines()

    new_lines = list()

    for line in lines:
        new_line = line
        match = re.match(HEADER_REGEXP, line)
        if match:
            header = match.groups()[0]
            if header in headers_dict:
                need_fix = True
                subdir = headers_dict[header]
                new_header = "<%s/%s>" % (subdir, header)
                new_line = re.sub(r"(\s*#\s*include\s+)[\"<]\s*%s[\">]" % (header),
                                  r"\1%s" % (new_header),
                                  line)
                sys.stdout.write("- " + line)
                sys.stdout.write("+ " + new_line)
                sys.stdout.write("\n")
        new_lines.append(new_line)
    if not need_fix:
        return

    if not write:
        return

    if backup:
        new_name = filename + ".back"
        shutil.copy(filename, new_name)

    with open(filename, "w") as fp:
        fp.writelines(new_lines)

def refactor_headers(naoqi_sdk, sources,
        backup=True,
        write=False):
    """ Refactor the headers for the given sources

    No changes will be made unless write is True
    A backup will be made for each modified file if backup is True
    """
    if not os.path.isdir(sources):
        raise Exception("%s is not a directory" % sources)

    headers_dict = parse_headers(naoqi_sdk)

    for root, directories, filenames in os.walk(sources):
        for filename in filenames:
            filename = os.path.join(root, filename)
            basename = os.path.basename(filename)
            if basename.startswith("."):
                continue
            if is_source(filename):
                fix_headers(filename, headers_dict, backup=backup, write=write)




def main():
    """ Parses command line arguments

    """
    parser = optparse.OptionParser()
    parser.add_option("-w", "--write",  action="store_true",
        help="Write back modified files")
    parser.add_option("-n", "--no-backup", action="store_false",
        dest="backup",
        help="Do not write any backup")
    parser.set_defaults(
        write = False,
        backup = True)

    (opts, args) = parser.parse_args()
    if len(args) != 2:
        print "Usage: refactor-headers /path/to/naoqi/sdk /path/to/sources"
        sys.exit(2)

    [naoqi_sdk, sources] = args
    sources = os.path.abspath(sources)

    refactor_headers(naoqi_sdk, sources,
        backup=opts.backup,
        write =opts.write)



if __name__ == "__main__":
    main()


