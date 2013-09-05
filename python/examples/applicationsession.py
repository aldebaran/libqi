##
## Copyright (C) 2012, 2013 Aldebaran Robotics
##

""" Typical usage of ApplicationSession
"""

import sys
import qi

def main():
    """
      ApplicationSession is an Application with an embedded session that you
      can get anytime using the session method. It automatically connects the
      session once start or run are called. It also does its own
      arguments-parsing. If you don't want either of those behaviors, you may
      prefer to use a regular Application and your own Session instead.
      The initialization takes a list as argument which correspond to the
      arguments given to the program. If the --qi-url is given to your program,
      the session will be connected to the given value later on. The same goes
      for --qi-listen-url option.
      If no url are given the default adress to connect to will be
      tcp://127.0.0.1:9559.
      Note : The parsed options will removed from the given list by Application.
    """

    # Initialize the ApplicationSession first.
    app = qi.ApplicationSession(sys.argv)

    # You can check what url the session will connect to with the url method.
    print "The url used by ApplicationSession will be " + app.url()

    # You can manipulate the session used by the application with the session
    # method. You must not use the connect method on it though since it will be
    # done automatically with either start or run method.
    session = app.session()

    # The start method connects the session and starts listening if the --qi-url
    # options was given. This call is optional though since the run method calls
    # it if it hasn't been done yet.
    app.start()

    # Calls the start method if needed and then run the application.
    app.run()

    # The session is now connected and available at that point.

if __name__ == "__main__":
    main()
