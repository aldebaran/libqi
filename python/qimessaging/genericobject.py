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

# If you're trying to find an object in this module, for example
# qimessaging.genericobject.ALMemory, then GenericObject is the class you're
# searching for. It's only renamed depending on the service it constructs on.
class GenericObject:

    def __init__(self, qi_object=None, qi_name=None):
        """ Object constructor. If qi_object is not set, create empty object."""

        def gom_factory(*args):
            class GenericObjectMethod:
                def __init__(self, go, name, signatures):
                    self._go = go

                    self.__name__ = name
                    self.__doc__ = "See __docs__ attribute for documentation per overload."

                    self.__signatures__ = signatures
                    self.__parameters__, self.__docs__ = dict(), dict()
                    for it, sig in enumerate(signatures):
                        a = name + '::(' + sig + ')'
                        b = _qim.qi_get_parameters_descriptions
                        self.__parameters__[sig] = [it.split(': ', 1) for it in b(self._go._obj, a)]
                        self.__docs__[sig] = _qim.qi_get_method_description(self._go._obj, a)
                        if self.__docs__[sig] is None:
                            self.__docs__[sig] = 'No documentation provided.'
                    self.__sigreturns__ = _qim.qi_get_sigreturn(self._go._obj, name).split(',')
                    self.__docreturns__ = _qim.qi_get_sigreturn_description(self._go._obj, name)

                def __call__(self, *args, **kwargs):
                    func = self.__name__
                    overload = kwargs.pop('overload', None)
                    if len(self.__signatures__) == 1:
                        func = '{name}::({sig})'.format(
                            name = self.__name__,
                            sig = self.__signatures__[0],
                        )
                    elif overload in self.__signatures__:
                        func = '{name}::({sig})'.format(
                            name = self.__name__,
                            sig = overload
                        )
                    return _qim.qi_generic_call(self._go._obj, func, args)
            return GenericObjectMethod(*args)

        if qi_object is None:
            self._obj = _qim.qi_object_create("obj")
        else:
            self._obj = qi_object
            if qi_name is not None:
                self.__class__.__name__ = qi_name

            self.__doc__ = _qim.qi_get_object_description(self._obj)

            # Add methods to self based on methods present.
            methods = _qim.qi_object_methods_vector(self._obj)
            def split_signatures(method):
                res = zip(*[it.rsplit('::', 1) for it in method.split(',')])
                return res[0][0], [it[1:-1] for it in res[1]]

            for method in methods:
                name, signatures = split_signatures(method)
                setattr(self, name, gom_factory(self, name, signatures))

    def __del__(self):
        """ Object destructor, also destroy C++ object.
        """
        if self._obj:
            _qim.qi_object_destroy(self._obj)
