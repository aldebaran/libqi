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
DEBUG   = VERBOSE

def _printToString(*args):
    return ' '.join(str(x) for x in args)

class PyLogger:
    def __init__(self, category):
        self.category = category

    def fatal(self, *args):
        info = logGetTraceInfo()
        pylog(FATAL, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def error(self, *args):
        info = logGetTraceInfo()
        pylog(ERROR, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def warning(self, *args):
        info = logGetTraceInfo()
        pylog(WARNING, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def info(self, *args):
        info = logGetTraceInfo()
        pylog(INFO, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def verbose(self, *args):
        info = logGetTraceInfo()
        pylog(VERBOSE, self.category, _printToString(*args), info.filename, info.function, info.lineno)

    def debug(self, *args):
        self.verbose(*args)

def getLogger( name):
    return PyLogger(name)

def logSilent(cat, *args):
    info = logGetTraceInfo()
    pylog(0, cat, _printToString(*args), info.filename, info.function, info.lineno)

def logFatal(cat, *args):
    info = logGetTraceInfo()
    pylog(FATAL, cat, _printToString(*args), info.filename, info.function, info.lineno)

def logError(cat, *args):
    info = logGetTraceInfo()
    pylog(ERROR, cat, _printToString(*args), info.filename, info.function, info.lineno)

def logWarning(cat, *args):
    info = logGetTraceInfo()
    pylog(WARNING, cat, _printToString(*args), info.filename, info.function, info.lineno)

def logInfo(cat, *args):
    info = logGetTraceInfo()
    pylog(INFO, cat, _printToString(*args), info.filename, info.function, info.lineno)

def logVerbose(cat, *args):
    info = logGetTraceInfo()
    pylog(VERBOSE, cat, _printToString(*args), info.filename, info.function, info.lineno)

def logDebug(cat, *args):
    logVerbose(cat, *args)
