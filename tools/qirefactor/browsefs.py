#!/usr/bin/env python
##
## browsefs.py
## Login : <ctaf@speedcore>
## Started on  Mon Aug  1 00:53:29 2011
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

DEV_DIR_BLACKLIST = ( "build*", ".build*", ".dep", # build directories
                      ".git", ".svn" )             # VCS

DEV_FILE_BLACKLIST = ( "*.back"  , "*~",          # backup
                       "*.py[co]",                # python
                       "*.[oa]",                  # gcc
                       "*.exe", "*.dll", "*.lib", # win32
                       "*.dylib",                 # mac
                       "*.so" )                   # linux

def fnmatch_list(name, globs):
    """
    return True if name match with at least one element of the list
    """
    for fo in globs:
        if fnmatch.fnmatch(name, fo):
            return True
    return False

def fnmatch_blwl(fname, blacklist = None, whitelist = None):
    """
    fnmath on fname using a blacklist and a whitelist
    blacklist and whitelist are optionnal.
    If both are None, then obviouly the fname match and True is returned

    return True on success
    """
    #filter out element that are blacklisted
    if blacklist and fnmatch_list(fname, blacklist):
        return False
    #filter out element that do not match the whitelist
    if whitelist and not fnmatch_list(fname, whitelist):
        return False
    return True

def foreach_file(root_directory,
                 file_action = None,
                 dir_action  = None,
                 file_blacklist = None,
                 file_whitelist = None,
                 dir_blacklist = None,
                 dir_whitelist = None):
    """
    Recusively traverse each files and directories of the root_directory.
    file_action is called on each file, dir_action is called on each dir.
    optional blacklist and whitelist filters can be applied on files and directories.

    return a tuple with the number of matches (folders, files)
    """
    dcount = 0
    fcount = 0

    for fname in os.listdir(directory):
        fpath = os.path.join(directory, fname)

        if os.path.isfile(fpath) and fnmatch_blwl(fname, file_blacklist, file_whitelist):
            fcount += 1
            if file_action:
                file_action(fpath)
        if os.path.isdir(fpath) and  fnmatch_blwl(fname, dir_blacklist, dir_whitelist):
            dcount += 1
            (dcount, fcount) += recurse_file(fpath, file_action   , dir_action    ,
                                                    file_blacklist, file_whitelist,
                                                    dir_blacklist , dir_whitelist)
            if dir_action:
                dir_action(fpath)
    return (dcount, fcount)

def backup_file(fpath, lines):
    """
    write lines in a backup. The backup is fpath-<i>.back
    where i start from 0 and is incremented until a file with that name exists.
    """
    i = 0
    while True:
        i += 1;
        back_file = "%s-%i.back" % (fpath, i)
        if os.path.exists(back_file):
            continue
        with f = open(back_file, "w"):
            f.writelines(lines)
        break
