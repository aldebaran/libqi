"""Pretty printers for libqi.
Add python execfile("/path/to/this/file") in your ~/.gdbinit or /etc/gdb/gdbinit
Your gdb need to be compile with python and version > 7.0
"""

import gdb
import gdb.types
import re

class QiFuturePrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        fut = self.val['_p']['px'].dereference()
        _fut = fut['_p']
        state = str(_fut['_state'].cast(gdb.lookup_type("qi::FutureState")))
        type = fut.type.template_argument(0).name
        if state == "qi::FutureState_Running":
            return "Running qi::Future of %s" % type
        elif state == "qi::FutureState_Canceled":
            return "Canceled qi::Future of %s" % type
        elif state == "qi::FutureState_FinishedWithError":
            return "qi::Future of %s finished with error %s" % (type, _fut['_error'])
        elif state == "qi::FutureState_FinishedWithValue":
            return "qi::Future of %s finished with value %s" % (type, fut['_value'])
        elif state == "qi::FutureState_None":
            return "qi::Future of %s not associated with a promise" % type

class QiBufferPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        buf = self.val['_p']['px'].dereference()
        return "qi::Buffer of length %i, capacity %i, sub-buffers %s" % (buf["used"], buf["available"], buf['_subBuffers'])

def lookup_type(val):
    type = str(gdb.types.get_basic_type(val.type))
    if type == 'qi::Buffer':
        return QiBufferPrinter(val)
    regex = re.compile("^qi::Future<.*>$")
    m = regex.match(type)
    if m:
        return QiFuturePrinter(val)
    return None

if __name__ == "__main__":
    gdb.pretty_printers.append(lookup_type)
