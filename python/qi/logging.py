##
## Author(s):
##  - Herve CUCHE <hcuche@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics
##

from _qi import pylog
from _qi import setLevel, setContext, setFilters
import inspect
import uuid

def logGetTraceInfo():
    callerframerecord = inspect.stack()[2]    # 0 represents this line
    frame = callerframerecord[0]
    info = inspect.getframeinfo(frame)
    return info

# Log Level
FATAL   = 1
ERROR   = 2
WARNING = 3
INFO    = 4
VERBOSE = 5
DEBUG   = 6

def _printToString(*args):
    return ' '.join(str(x) for x in args)

class Logger:
    def __init__(self, category):
        self.category = category

    def fatal(self, *args):
        """ generate a fatal log message """
        info = logGetTraceInfo()
        pylog(FATAL, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def error(self, *args):
        """ generate an error log message """
        info = logGetTraceInfo()
        pylog(ERROR, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def warning(self, *args):
        """ generate a warning log message """
        info = logGetTraceInfo()
        pylog(WARNING, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def info(self, *args):
        """ generate an info log message """
        info = logGetTraceInfo()
        pylog(INFO, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def verbose(self, *args):
        """ generate a verbose log message """
        info = logGetTraceInfo()
        pylog(VERBOSE, self.category, _printToString(*args), info.filename, info.function, info.lineno)

def fatal(cat, *args):
    """ generate a fatal log message """
    info = logGetTraceInfo()
    pylog(FATAL, cat, _printToString(*args), info.filename, info.function, info.lineno)

def error(cat, *args):
    """ generate a error log message """
    info = logGetTraceInfo()
    pylog(ERROR, cat, _printToString(*args), info.filename, info.function, info.lineno)

def warning(cat, *args):
    """ generate a warning log message """
    info = logGetTraceInfo()
    pylog(WARNING, cat, _printToString(*args), info.filename, info.function, info.lineno)

def info(cat, *args):
    """ generate a info log message """
    info = logGetTraceInfo()
    pylog(INFO, cat, _printToString(*args), info.filename, info.function, info.lineno)

def verbose(cat, *args):
    """ generate a verbose log message """
    info = logGetTraceInfo()
    pylog(VERBOSE, cat, _printToString(*args), info.filename, info.function, info.lineno)
