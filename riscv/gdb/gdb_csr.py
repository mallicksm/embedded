import gdb

# ----------------------------------------
# Helpers
# ----------------------------------------

def csr(name):
   try:
      # force unsigned 32-bit
      return int(gdb.parse_and_eval(f"${name}")) & 0xffffffff
   except:
      return None

def sym(addr):
   if addr is None:
      return "<na>"

   try:
      out = gdb.execute(f"info symbol 0x{addr:x}", to_string=True).strip()
      return out.split("\n")[0]
   except:
      return "<unknown>"

def decode_mcause(val):
   if val is None:
      return ""

   irq  = (val >> 31) & 1
   code = val & 0xff

   return f"(irq={irq} code={code})"

def pr(name, val, show_sym=False, decode=None):
   if val is None:
      print(f"{name:10} : <na>")
      return

   if show_sym:
      print(f"{name:10} : 0x{val:08x}  ({sym(val)})")
   else:
      print(f"{name:10} : 0x{val:08x}")

   if decode:
      print(" " * 12 + decode(val))


# ----------------------------------------
# Command
# ----------------------------------------

class DumpCSR(gdb.Command):
   def __init__(self):
      super().__init__("dumpcsr", gdb.COMMAND_USER)

   def invoke(self, arg, from_tty):
      print("\n=== CSR: trap ===")

      pr("mepc",   csr("mepc"),   show_sym=True)
      pr("mcause", csr("mcause"), decode=decode_mcause)
      pr("mtval",  csr("mtval"))
      pr("mtvec",  csr("mtvec"),  show_sym=True)

      print("\n=== CSR: state ===")
      pr("mstatus", csr("mstatus"))
      pr("misa",    csr("misa"))

      print("\n=== CSR: interrupt ===")
      pr("mie", csr("mie"))
      pr("mip", csr("mip"))

      print("\n=== CSR: misc ===")
      pr("mscratch", csr("mscratch"))

      print("\n=================\n")


# ----------------------------------------
# Register command
# ----------------------------------------

DumpCSR()
