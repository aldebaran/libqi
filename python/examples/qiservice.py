#!/usr/bin/python2
##
## Copyright (C) 2012, 2013 Aldebaran Robotics
##

""" Python service providing the famous 'reply::s(s)' function
"""

import sys
import time
import qi
import threading

def makeIt(p):
    time.sleep(1)
    print "PAFFF"
    p.setValue(42)

class ServiceTest:
    def __init__(self):
        self.onFoo = qi.Signal("(i)")
        self.testEvent = qi.Signal("(s)")
        self.testEventGeneric = qi.Signal()

    def reply(self, plaf):
        print "v:", plaf
        return plaf + "bim"

    def error(self):
        d= dict()
        print "I Will throw"
        r = d['pleaseraise']

    def fut(self):
        p = qi.Promise()
        #p.setValue(42)
        threading.Thread(target=makeIt, args=[p]).run()
        return p.future()

    @qi.nobind
    def nothing(self):
        print "nothing"
        pass

    @qi.bind(qi.String, (qi.String, qi.Int), "plik")
    def plok(self, name, index):
        print "ploK"
        return name[index]

def get_servicedirectory_address():
    """ Parse command line arguments

    Print usage is service directory address is not set.
    """
    if len(sys.argv) != 2:
        print 'Usage : %s qi-service.py directory-address' % sys.argv[0]
        print 'Assuming service directory address is tcp://127.0.0.1:9559'
        return "tcp://127.0.0.1:9559"

    return sys.argv[1]

def main():
    """ Entry point of qiservice
    """
    #1 Check if user give us service directory address.
    sd_addr = get_servicedirectory_address()

    s = qi.Session()
    s.connect(sd_addr)
    m = ServiceTest()
    s.registerService("serviceTest", m)

    #5 Call Application.run() to join event loop.
    i = 0
    while True:
      mystr = "bim" + str(i)
      print "posting:", mystr
      myplouf = [ "bim", 42 ]
      m.testEvent(mystr)
      m.testEventGeneric(myplouf)
      time.sleep(1);
      i += 1

    #6 Clean
    s.close()
    #main : Done.

if __name__ == "__main__":
    main()
