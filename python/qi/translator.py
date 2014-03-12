##
## Copyright (C) 2014 Aldebaran Robotics
##

from _qi import Translator
import os
from .logging import warning

globTranslator = None

def defaultTranslator(name):
    global globTranslator
    if globTranslator:
        return globTranslator

    globTranslator = Translator(name)
    return globTranslator

def tr(msg, domain=None, locale=None):
    global globTranslator
    if not globTranslator:
        warning("qi.translator", "You must init your translator first.")
        return msg
    if domain is None:
        return globTranslator.translate(msg)
    if locale is None:
        return globTranslator.translate(msg, domain)
    return globTranslator.translate(msg, domain, locale)

__all__ = ( "defaultTranslator", "tr" )
