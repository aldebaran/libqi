##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

""" QiMessaging object class
"""

import _qipy

__all__ = [ "GenericObject", ]

class MetaMethod:
    def __init__(self, name, cppMetaMethod):
        self.methodId = cppMetaMethod[0]
        self.description = cppMetaMethod[3]
        self.name = name
        self.signature = cppMetaMethod[2]
        self.returnSignature = cppMetaMethod[1]
        self.returnDescription = cppMetaMethod[5]
        self.parameters = cppMetaMethod[4]

class MetaObject:
    def __init__(self, cppMetaObject):
        self.methods = dict()
        self.events = cppMetaObject[1]
        self.description = cppMetaObject[2]

        for k,v in cppMetaObject[0].iteritems():
            name = v[2].split('::', 1)[0]
            r = self.methods.get(name, list())
            r.append(MetaMethod(name, v))
            self.methods[name] = r

class GenericObjectMethod:
    def __init__(self, parent, name, metamethod):
        self._parent = parent
        self.__name__ = name
        self.__doc__ = "See __docs__ attribute for documentation per overload."
        #TODO: set attribute here

    def __call__(self, *args, **kwargs):
        func = self.__name__
        async = kwargs.pop('async', False)
        overload = kwargs.pop('overload', None)
        if overload:
            func = '%s::(%s)' % (self.__name__, overload)
        print "calling method:", func
        return _qipy.qipy_object_call(self._parent._obj, func, args)

class GenericObject:

    def __init__(self, qi_object=None, qi_name=None):
        """ Object constructor. If qi_object is not set, create empty object."""
        print "Go CTOR"
        #TODO: should we?
        if qi_object is None:
            self._obj = _qipy.qi_object_create()
            return

        self._obj = qi_object
        if qi_name is not None:
            self.__class__.__name__ = qi_name

        self.__metaobject__ = MetaObject(_qipy.qipy_object_get_metaobject(self._obj))
        self.__doc__ = self.__metaobject__.description

        # Add methods to self based on methods present.
        for k,v in self.__metaobject__.methods.iteritems():
            setattr(self, k, GenericObjectMethod(self, k, v))

    def __del__(self):
        """ Object destructor, also destroy C++ object.
        """
        if self._obj:
            _qipy.qi_object_destroy(self._obj)
