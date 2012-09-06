##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import sys
import qimessaging

from qimessaging import Application
from qimessaging import Session

def init_session(sd_addr):

    session = Session()
    try:
        session.connect(sd_addr)
    except qimessaging.ConnectionError as e:
        print e
        return None

    if not session.wait_for_connected():
        print "Cannot connect to service directory"
        return None

    return session

def get_service(session, service_name):
    obj = session.service(service_name)

    if obj is None:
        print "Oops, cannot get service serviceTest"
        return None

    return obj


def call_reply(obj):
    print "Synchronous call :"

    try:
        value = obj.call("reply::(s)", "titi")
    except qimessaging.CallError as e:
        print e
        return

    print 'Reply : %s' % value

def get_servicedirectory_address(argv):
    if len(argv) != 2:
        print 'Usage : python2 qi-client.py directory-address'
        print 'Assuming service directory address is tcp://127.0.0.1:5555'
        return "tcp://127.0.0.1:5555"

    return argv[1]

def  main(argv):
    sd_addr = get_servicedirectory_address(argv)

    session = init_session(sd_addr)
    if not session:
        sys.exit()

    obj = get_service(session, "serviceTest")
    if not obj:
        sys.exit()

    call_reply(obj)

    session.close()
    sys.exit()

if __name__ == "__main__":
    app = Application(sys.argv)
    main(sys.argv)
