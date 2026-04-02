import gdb

class Uint32Printer:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "0x%08x" % int(self.val)

def lookup(val):
    t = val.type.strip_typedefs()

    # match 32-bit unsigned ints
    if t.code == gdb.TYPE_CODE_INT and t.sizeof == 4:
        if not t.is_signed:
            return Uint32Printer(val)

    return None

gdb.pretty_printers.append(lookup)
