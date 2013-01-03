##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

""" QiMessaging object class
"""

import _qimessagingswig as _qim

class CallError(Exception):
    """ CallError exception raised by Object.call() method
    """
    def __init__(self, value):
        """ Constructor of CallError exception
        """
        self._value = value

    def __str__(self):
        """ Getter on error value, Python style.
        """
        return str(self._value)


class SerializationError(Exception):
    def __init__(self, value):
        self._value = value

    def __str__(self):
        return str(self._value)


class GenericObject:
    """ Main class of QiMessaging
    """
    def __init__(self, qi_object = None):
        """ Object constructor.

        If qi_object is not set, create empty object.
        """
        def innerfunc_factory(name, signatures):
            def innerfunc(*args, **kwargs):
                func = innerfunc.__name__
                overload = kwargs.pop('overload', None)
                if len(innerfunc.__signatures__) == 1:
                    func = '{name}::({sig})'.format(
                        name = innerfunc.__name__,
                        sig = innerfunc.__signatures__[0],
                    )
                elif overload in innerfunc.__signatures__:
                    func = '{name}::({sig})'.format(
                        name = innerfunc.__name__,
                        sig = overload
                    )
                return _qim.qi_generic_call(self._obj, name, args)
            innerfunc.__name__ = name
            innerfunc.__doc__ = "Docstring of %s" % name
            innerfunc.__signatures__ = signatures
            innerfunc.__sigreturns__ = _qim.qi_get_sigreturn(self._obj, name).split(',')
            return innerfunc

        if qi_object is None:
            self._obj = _qim.qi_object_create("obj")
        else:
            self._obj = qi_object

            # Add methods to self based on methods present.
            methods = _qim.qi_object_methods_vector(self._obj)
            def split_signatures(method):
                res = zip(*[it.rsplit('::', 1) for it in method.split(',')])
                return res[0][0], [it[1:-1] for it in res[1]]

            for method in methods:
                name, signatures = split_signatures(method)
                setattr(self, name, innerfunc_factory(name, signatures))

    def __del__(self):
        """ Object destructor, also destroy C++ object.
        """
        if self._obj:
            _qim.qi_object_destroy(self._obj)
