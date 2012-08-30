#!/usr/bin/env python
##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import qi

def callReply(sdAddr):
    print("Connecting to serviceTest")

    session = Session()
    session.connect(sdAddr)

    if session.waitForConnected(1000) == False:
        print("Cannot connect to service directory")
        return 1

    obj = session.service("serviceTest")
    if obj == 0:
        print("Oops, cannot get service serviceTest")
        return 1

    message = Message()
    message.writeString("plaf")
    future = obj.call("reply::s(s)", message)

    future.wait();

    if future.isError() == True:
        print("An error occured :", future.getError())
        return 1

    if future.isReady() == True and future.isError() == False:
        answer = future.getValue()
        value = answer.readString();
        print("Reply :", value)

    session.disconnect()
    return 0

def main():
    app = Application()
    print("qi-client.py")
    # Todo : Parse argument
    print("Assuming master-address is tcp://127.0.0.1:5555")
    sdAddr = "tcp://127.0.0.1:5555"
    return callReply(sdAddr)

if __name__ == "__main__":
    main()
