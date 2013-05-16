##
## Copyright (C) 2012 Aldebaran Robotics
##

""" Python service providing the famous 'reply::s(s)' function
"""

import sys
import time
import qi

class ServiceTest:
    def __init__(self):
        self.onFoo = qi.Signal("i")
        self.onBar = qi.Signal("i")
        self.testEvent = qi.Signal("(s)")


    def reply(self, plaf):
        print "iciiii"
        print "v:", plaf
        return plaf + "bim"

    def rep(self):
        self.onFoo(32)
        print "rep"

    def totot(self, pif, paf):
        print "laaaa"
        pass

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
    #0 Declare app
    app = qi.Application()

    #1 Check if user give us service directory address.
    sd_addr = get_servicedirectory_address()

    s = qi.Session()

    s.connect(sd_addr)
    m = ServiceTest()
    s.register_service("serviceTest", m)

    #5 Call Application.run() to join event loop.
    i = 0
    while True:
      mystr = "bim" + str(i)
      print "posting:", mystr
      m.testEvent(mystr)
      time.sleep(1);
      i += 1

    #6 Clean
    s.close()
    app.stop()
    #main : Done.

if __name__ == "__main__":
    main()
