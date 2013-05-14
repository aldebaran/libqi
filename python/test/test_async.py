#! /usr/bin/python2

import qi
import time

def my_callback(f):
    print f.value()

def funky(o):
    f = o.services(_async = True)
    f.add_callback(my_callback)
    return

def main():
    sd = qi.ServiceDirectory()
    sd.listen('tcp://127.0.0.1:5555')
    s = qi.Session()
    s.connect('tcp://127.0.0.1:5555')
    o = s.service('ServiceDirectory')
    funky(o)
    time.sleep(2)

if __name__ == "__main__":
    main()
