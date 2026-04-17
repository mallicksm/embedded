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
        pairs = [
            ("ra", "sp"),
            ("s0", "s1"),
            ("s2", "s3"),
            ("s4", "s5"),
            ("s6", "s7"),
            ("s8", "s9"),
            ("s10", "s11"),
        ]

        for a, b in pairs:
            yield (
                f"{a},{b}",
                f"{self.val[a]}  {self.val[b]}"
            )
# --------------------------------------------------
# trapframe printer → structured registers
# --------------------------------------------------
class TrapFramePrinter:
   def __init__(self, val):
      self.val = val

   def to_string(self):
      return "trapframe"

   def children(self):
      pairs = [
         # control
         ("ra", "sp"),
         ("gp", "tp"),

         # temporaries
         ("t0", "t1"),
         ("t2", "t3"),
         ("t4", "t5"),
         ("t6", None),

         # arguments
         ("a0", "a1"),
         ("a2", "a3"),
         ("a4", "a5"),
         ("a6", "a7"),

         # callee-saved (only if you still have them in tf)
         ("s0", "s1"),
         ("s2", "s3"),
         ("s4", "s5"),
         ("s6", "s7"),
         ("s8", "s9"),
         ("s10", "s11"),

         # control regs (if present in struct)
         ("mepc", None),
      ]

      for a, b in pairs:
         try:
            if b is None:
               yield (f"{a}", f"{self.val[a]}")
            else:
               yield (
                  f"{a},{b}",
                  f"{self.val[a]}  {self.val[b]}"
               )
         except:
            yield (f"{a}", "<na>")

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

    # ---- trapframe match ----
    if str(t) == "struct trapframe":
        return TrapFramePrinter(val)

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
