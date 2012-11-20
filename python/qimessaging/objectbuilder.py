##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

""" Provide QiMessaging object manipulation and creation methods.

ObjectBuilder helps user to create service object template.
To create an object instance : ObjectBuilder.object()
"""

import _qimessagingswig
import qimessaging.binder as binder
from qimessaging.genericobject import GenericObject

class ObjectBuilder:
    """ Object building facility class.
    """
    def __init__(self):
        """ Allocate and initialise a QiMessaging object builder
        """
        self._builder = _qimessagingswig.qi_object_builder_create()

    def object(self):
        """ Allocate and return QiMessaging object instance from object builder.

        Returns:
            Object on sucess, None on error
        """
        obj = _qimessagingswig.qi_object_builder_get_object(self._builder)

        if not obj:
            return None
        return GenericObject(obj)

    def register_method(self, method):
        """ Register given function on object builder.
        """
        signature = binder.getFunctionSignature(method)
        if (signature is None):
          print "No signature can be found for this method"
          return
        _qimessagingswig.qi_bind_method(self._builder, signature, method)

    def __del__(self):
        """ Destroy related object builder.
        """
        _qimessagingswig.qi_object_builder_destroy(self._builder)
