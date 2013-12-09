##
## Author(s):
##  - Herve CUCHE <hcuche@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics
##

from _qi import pylog
from _qi import pysetlevel
from _qi import pysetcontext
from _qi import pysetfilters
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

def setLevel(level):
    pysetlevel(level)

def setContext(context):
    pysetcontext(context)

def setFilters(filters):
    pysetfilters(filters)

class PyLogger:
    def __init__(self, category):
        self.category = category

    def fatal(self, message):
        info = logGetTraceInfo()
        pylog(1, self.category, message, info.filename, info.function, info.lineno)

    def error(self, message):
        info = logGetTraceInfo()
        pylog(2, self.category, message, info.filename, info.function, info.lineno)

    def warning(self, message):
        info = logGetTraceInfo()
        pylog(3, self.category, message, info.filename, info.function, info.lineno)

    def info(self, message):
        info = logGetTraceInfo()
        pylog(4, self.category, message, info.filename, info.function, info.lineno)

    def debug(self, message):
        info = logGetTraceInfo()
        pylog(5, self.category, message, info.filename, info.function, info.lineno)

def getLogger( name):
    return PyLogger(name)

def logSilent(cat, msg):
    info = logGetTraceInfo()
    pylog(0, cat, msg, info.filename, info.function, info.lineno)

def logFatal(cat, msg):
    info = logGetTraceInfo()
    pylog(1, cat, msg, info.filename, info.function, info.lineno)

def logError(cat, msg):
    info = logGetTraceInfo()
    pylog(2, cat, msg, info.filename, info.function, info.lineno)

def logWarning(cat, msg):
    info = logGetTraceInfo()
    pylog(3, cat, msg, info.filename, info.function, info.lineno)

def logInfo(cat, msg):
    info = logGetTraceInfo()
    pylog(4, cat, msg, info.filename, info.function, info.lineno)

def logVerbose(cat, msg):
    info = logGetTraceInfo()
    pylog(5, cat, msg, info.filename, info.function, info.lineno)

def logDebug(cat, msg):
    info = logGetTraceInfo()
    pylog(6, cat, msg, info.filename, info.function, info.lineno)
