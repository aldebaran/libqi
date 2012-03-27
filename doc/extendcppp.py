#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2011, 2012 Aldebaran Robotics
##

from sphinx.locale import l_, _
from sphinx.domains.cpp import CPPDomain
from sphinx.util.docfields import Field, TypedField

class CPPDomainWithParam(CPPDomain):

    #we add parameters and returntype to cpp functions
    def __init__(self, *args, **kargs):
        CPPDomain.__init__(self, *args, **kargs)
        self.directives['function'].doc_field_types = [
            TypedField('parameter', label=l_('Parameters'),
                       names=('param', 'parameter', 'arg', 'argument'),
                       typerolename='type', typenames=('type',)),
            Field('returnvalue', label=l_('Returns'), has_arg=False,
                  names=('returns', 'return')),
            Field('returntype', label=l_('Return type'), has_arg=False,
                  names=('rtype',)),
            ]

def setup(app):
    app.override_domain(CPPDomainWithParam)
