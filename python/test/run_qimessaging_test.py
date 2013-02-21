""" Helper script to run qimessaging tests

"""

import os
import sys
import argparse
import importlib

def fix_sys_path(sdk_dir, src_dir):
    """ Make it possible to import qimessaging and the tests """
    this_dir = os.path.basename(__file__)
    sys.path.insert(0, src_dir)
    sys.path.insert(0, os.path.join(src_dir, "test"))
    sdk_lib = os.path.join(sdk_dir, "lib")
    sys.path.insert(0, sdk_lib)

def run_test(test_name):
    """ Run a test given its name """
    module = importlib.import_module(test_name)
    module.main()

def main():
    """ Main entry point """
    parser = argparse.ArgumentParser()
    parser.add_argument("--sdk-dir", required=True)
    parser.add_argument("--src-dir", required=True)
    parser.add_argument("test_name")
    args = parser.parse_args()
    fix_sys_path(args.sdk_dir, args.src_dir)
    run_test(args.test_name)

if __name__ == "__main__":
    main()
