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

    qi.logFatal("test.logger", "log fatal", 1)
    qi.logError("test.logger", "log error", 1)
    qi.logWarning("test.logger", "log warning", 1)
    qi.logInfo("test.logger", "log info", 1)
    qi.logVerbose("test.logger", "log verbose", 1)
    qi.logDebug("test.logger",  "log debug", 1)

def test_loggingLevel():
    logger = qi.logging.getLogger("test.logging")
    qi.logging.setContext(254)
    qi.logging.setLevel(qi.logging.FATAL)
    logger.fatal("log fatal")
    logger.error("log error")
    logger.warning("log warning")
    logger.info("log info")
    logger.debug("log debug")

    logger.fatal("log fatal", 1)
    logger.error("log error", 1)
    logger.warning("log warning", 1)
    logger.info("log info", 1)
    logger.debug("log debug", 1)

def test_loggingFilters():
    logger = qi.logging.getLogger("test.logging")
    qi.logging.setContext(254)
    qi.logging.setFilters("+test*=2")
    logger.fatal("log fatal")
    logger.error("log error")
    logger.warning("log warning")
    logger.info("log info")
    logger.debug("log debug")

    logger.fatal("log fatal", 1)
    logger.error("log error", 1)
    logger.warning("log warning", 1)
    logger.info("log info", 1)
    logger.debug("log debug", 1)

def main():
    test_directlog()
    test_loggingLevel()
    test_loggingFilters()

if __name__ == "__main__":
    main()
