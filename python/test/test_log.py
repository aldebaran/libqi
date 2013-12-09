#!/usr/bin/env python

import qi
import qi.logging

def test_directlog():
    qi.logFatal("test.logger", "log fatal")
    qi.logError("test.logger", "log error")
    qi.logWarning("test.logger", "log warning")
    qi.logInfo("test.logger", "log info")
    qi.logVerbose("test.logger", "log verbose")
    qi.logDebug("test.logger",  "log debug")


def test_loggingLevel():
    logger = qi.logging.getLogger("test.logging")
    qi.logging.setContext(254)
    qi.logging.setLevel(qi.logging.FATAL)
    logger.fatal("log fatal")
    logger.error("log error")
    logger.warning("log warning")
    logger.info("log info")
    logger.debug("log debug")

def test_loggingFilters():
    logger = qi.logging.getLogger("test.logging")
    qi.logging.setContext(254)
    qi.logging.setFilters("+test*=2")
    logger.fatal("log fatal")
    logger.error("log error")
    logger.warning("log warning")
    logger.info("log info")
    logger.debug("log debug")

def main():
    test_directlog()
    test_loggingLevel()
    test_loggingFilters()

if __name__ == "__main__":
    main()
