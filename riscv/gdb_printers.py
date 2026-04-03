# gdb_printers.py
try:
    import gdb
except ImportError:
    gdb = None

# --------------------------------------------------
# uint32_t printer → hex
# --------------------------------------------------
class Uint32Printer:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "0x%08x" % int(self.val)

# --------------------------------------------------
# thread_context printer → structured registers
# --------------------------------------------------
class ThreadContextPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "thread_context"

    def children(self):
        # Order matters for debugging
        regs = [
            "sp", "ra",   # 🔥 most important first
            "s0","s1","s2","s3","s4","s5",
            "s6","s7","s8","s9","s10","s11"
        ]

        for r in regs:
            yield (r, self.val[r])

# --------------------------------------------------
# lookup dispatcher
# --------------------------------------------------
def lookup(val):
    if gdb is None:
        return None

    t = val.type.strip_typedefs()

    # ---- thread_context match ----
    if str(t) == "struct thread_context":
        return ThreadContextPrinter(val)

    # ---- uint32_t / unsigned 32-bit ----
    if t.code == gdb.TYPE_CODE_INT and t.sizeof == 4:
        if not t.is_signed:
            return Uint32Printer(val)

    return None

# --------------------------------------------------
# register printer
# --------------------------------------------------
def register_printers():
    if gdb is None:
        return

    # Avoid duplicate registration
    if lookup not in gdb.pretty_printers:
        gdb.pretty_printers.append(lookup)

# auto-register
register_printers()
