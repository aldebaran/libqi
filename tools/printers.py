"""Pretty printers for libqi.
Add python execfile("/path/to/this/file") in your ~/.gdbinit or /etc/gdb/gdbinit
Your gdb need to be compile with python and version > 7.0
"""

import gdb
import gdb.types

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
    return None

if __name__ == "__main__":
    gdb.pretty_printers.append(lookup_type)
