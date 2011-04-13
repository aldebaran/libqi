"""Fix headers :
#include "alproxy.h" -> #include <alproxy/alproxy.h>

"""

__usage__ = "trawl SOURCE_DIR"


header_exts = [".h", ".hpp", ".hh", ".hxx"]
src_exts    = [".cpp", ".c"]

import re
import os, sys, glob
from os.path import join

class file_object():
  def __init__(self):
    self.name = ""
    self.path = ""
    self.includes = []

def is_header(f):
    for ext in header_exts:
        if f.endswith(ext):
            return True
    return False

def is_src(f):
    for ext in src_exts:
        if f.endswith(ext):
            return True
    return False

def is_interesting(f):
    if is_header(f) or is_src(f):
      return True
    return False

def build_trees(top_src_dir):
    """Build a a list of trees of included files:
    by walking through the sources, and creating
    one file_object per file, that lists the headers
    """
    res = []
    for root, dirs, files in os.walk(top_src_dir):
        rel_root = os.path.relpath(root, top_src_dir)
        if "build" in rel_root:
            continue
        headers = [f for f in files if is_interesting(f)]
        for header in headers:
            file_obj = file_object()
            file_obj.name = header
            file_obj.path = os.path.normpath(os.path.join(rel_root, header))
            file_obj.includes = find_includes(os.path.join(top_src_dir, file_obj.path))
            res.append(file_obj)
    return res

def find_includes(path_to_file):
  includes = []
  o = open(path_to_file, "rb")
  for line in o:
    m = re.search('#[ ]*include[ ]*["<]+([^>"]+)[">]+', line)
    if (not m is None):
      includes.append(os.path.normpath(m.group(1)))
  o.close()
  return includes

def is_included(path_to_file, trees):
  for t in trees:
    for i in t.includes:
      if (i == path_to_file):
        return True
  return False

def print_trees(filter, trees):
  filtered_trees = filter_trees(filter, trees)
  for t in filtered_trees:
    print "==============================="
    if not is_src(t.name):
      if not is_included(t.path, trees):
        print "********** Unused ***********"
    print_tree("", t, trees)

def print_tree(indent, tree, trees):
  print indent + tree.path
  for i in tree.includes:
    # see if we know this file
    tt = find_file_object(i, trees)
    if tt is None:
      # An external header
      print indent + "  " + i
    else:
      # A known header
      print_tree(indent + "  ", tt, trees)


def find_file_object(path, trees):
  for t in trees:
    if t.path == path:
      return t
    if t.path.find(path) > -1:
      print "Warning bad include path for:"
  return None

def filter_trees(filter, trees):
  if filter is None:
    return trees
  filtered_trees = []
  for t in trees:
    if t.path.find(filter) > -1:
      filtered_trees.append(t)
  return filtered_trees

def main():
    """Parse command line"""
    if len(sys.argv) < 1:
        print "Usage: ", __usage__
        sys.exit(2)

    topdir = sys.argv[1]
    trees = build_trees(topdir)
    filter = None

    if (len(sys.argv) > 2):
        if sys.argv[2] == "--showlibs":
            #print_libs(trees)
            pass
        else:
            filter = sys.argv[2]
            print_trees(filter, trees)
    else:
        print_trees(filter, trees)

if __name__ == "__main__":
    main()
