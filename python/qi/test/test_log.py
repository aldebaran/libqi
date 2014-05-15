#!/usr/bin/env python

import qi
import qi.logging

def test_directlog():
    qi.fatal("test.logger", "log fatal")
    qi.error("test.logger", "log error")
    qi.warning("test.logger", "log warning")
    qi.info("test.logger", "log info")
    qi.verbose("test.logger", "log verbose")

    qi.fatal("test.logger", "log fatal", 1)
    qi.error("test.logger", "log error", 1)
    qi.warning("test.logger", "log warning", 1)
    qi.info("test.logger", "log info", 1)
    qi.verbose("test.logger", "log verbose", 1)

def test_loggingLevel():
    logger = qi.logging.Logger("test.logging")
    qi.logging.setContext(254)
    qi.logging.setLevel(qi.logging.FATAL)
    logger.fatal("log fatal")
    logger.error("log error")
    logger.warning("log warning")
    logger.info("log info")

    logger.fatal("log fatal", 1)
    logger.error("log error", 1)
    logger.warning("log warning", 1)
    logger.info("log info", 1)

def test_loggingFilters():
    logger = qi.logging.Logger("test.logging")
    qi.logging.setContext(254)
    qi.logging.setFilters("+test*=2")
    logger.fatal("log fatal")
    logger.error("log error")
    logger.warning("log warning")
    logger.info("log info")

    logger.fatal("log fatal", 1)
    logger.error("log error", 1)
    logger.warning("log warning", 1)
    logger.info("log info", 1)

def main():
    test_directlog()
    test_loggingLevel()
    test_loggingFilters()

if __name__ == "__main__":
    main()
