"""show headers :
Shows who includes who
"""

__usage__ = """
showdepends SOURCE_DIR filter
e.g
showdepends libs ALLIB
"""

cmake_exts = ["CMakeLists.txt"]

import re
import os, sys, glob
from os.path import join

class proj_object():
    def __init__(self):
        self.name = ""
        self.path = ""
        self.depends = []

def is_cmake(f):
    for ext in cmake_exts:
        if f.endswith(ext):
            return True
    return False

def build_trees(top_src_dir):
    """Build a a list of trees of included deps:
    by walking through the sources, and creating
    one proj_object per project, that lists the depends
    """
    res = []
    for root, dirs, files in os.walk(top_src_dir):
        rel_root = os.path.relpath(root, top_src_dir)
        if "build" in rel_root:
            continue
        cmake_files = [f for f in files if is_cmake(f)]
        for cmake_file in cmake_files:
			path = os.path.normpath(os.path.join(rel_root, cmake_file))
			res = res + build_proj_objects(os.path.join(top_src_dir, path ))
    return res

def build_proj_objects(path_to_file):
	#print "==========================="
	#print path_to_file
	projects = []
	o = open(path_to_file, "rb")
	whole_file = o.read()
	p = re.compile(r'[\s]*use_lib[\s]*[(]+[\s]*(?P<proj>[\w]+)[\s]+(?P<deps>[^)]+)[)]+', re.MULTILINE)
	iter = p.finditer(whole_file)
	for m in iter:
		#print "Proj:", m.group("proj")
		#print "AllDeps:", m.group("deps")
		splitre = re.compile(r'[\\r\\n\W]+', re.MULTILINE)
		deps = splitre.split(m.group("deps"))
		#print "SplitDeps:", deps
		proj_obj = proj_object()
		proj_obj.name = m.group("proj").upper()
		proj_obj.path = path_to_file
		for d in deps:
			if d is not "":
				#print "Dep:", d
				proj_obj.depends.append(d)
		projects.append(proj_obj)
		#print "--"
	o.close()
	return merge_projects(projects)

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
        print_tree("", t, trees)

def print_tree(indent, tree, trees):
	print indent + tree.name
	for i in tree.depends:
		# see if we know this file
		tt = find_proj(i, trees)
		if tt is None:
			if i == "ALL" or i == "REQUIRED":
				print indent + "  " + i + " WARNING " + tree.name
			else:
				# An external header
				print indent + "  " + i + "*"
		else:
			# A known header
			print_tree(indent + "  ", tt, trees)

def find_proj(path, trees):
    for t in trees:
        if t.name == path:
            return t
        if t.path.find(path) > -1:
            print "Warning bad include path for:"
    return None

def contains_proj(trees, name):
	for t in trees:
		if t.name == name:
			return True
	return False

def merge_projects(trees):
	new_trees = []
	for i in range(0, len(trees)):
		name = trees[i].name
		if not contains_proj(new_trees, name):
			if i < len(trees)-1:
				for k in range(i+1, len(trees)):
					if trees[k].name == name:
						trees[i].depends = trees[i].depends + trees[k].depends
			new_trees.append(trees[i])
	return new_trees

def filter_trees(filter, trees):
    if filter is None:
        return trees
    filtered_trees = []
    for t in trees:
        if t.name.find(filter) > -1:
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
        filter = sys.argv[2]
    print_trees(filter, trees)

if __name__ == "__main__":
    main()
