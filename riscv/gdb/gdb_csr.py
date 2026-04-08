import gdb
import os
from gdb_common import load_yaml, print_reg_yaml

# ---- helpers ----

def csr(name):
   try:
      return int(gdb.parse_and_eval(f"${name}")) & 0xffffffff
   except:
      return None

def sym(addr):
   if addr is None:
      return "<na>"

   try:
      out = gdb.execute("info symbol 0x%x" % addr, to_string=True).strip()

      if "No symbol matches" in out:
         return "<nosym>"

      return out.split("\n")[0]

   except:
      return "<unknown>"

def pr(name, val, show_sym=False, decode=None):
   if val is None:
      print("%-10s : <na>" % name)
      return

   if show_sym:
      print("%-10s : 0x%08x  (%s)" % (name, val, sym(val)))
   else:
      print("%-10s : 0x%08x" % (name, val))

   if decode:
      print(" " * 12 + decode(val))


# ---- yaml db ----
_db = load_yaml(os.path.join(os.path.dirname(__file__), "csr.yaml"))

# ---- command ----
class gdb_csr(gdb.Command):
   def __init__(self):
      super().__init__("gdb_csr", gdb.COMMAND_USER)

   def dump_regs(self, regs_sym, regs_yaml):
      regs = regs_sym + regs_yaml

      for r in regs:
         v = csr(r)

         if r in regs_yaml and r in _db:
            print_reg_yaml(_db, r, v)
         else:
            pr(r, v, show_sym=(r in regs_sym))

   def invoke(self, arg, from_tty):
      print("\n=== CSR: trap ===")
      self.dump_regs(
         regs_sym  = ["mepc", "mtvec"],
         regs_yaml = ["mcause", "mtval"]
      )
      
      print("\n=== CSR: state ===")
      self.dump_regs(
         regs_sym  = [],
         regs_yaml = ["mstatus", "misa"]
      )
      
      print("\n=== CSR: interrupt ===")
      self.dump_regs(
         regs_sym  = [],
         regs_yaml = ["mie", "mip"]
      )
      
      print("\n=== CSR: misc ===")
      self.dump_regs(
         regs_sym  = ["mscratch"],
         regs_yaml = []
      )
      print("\n=================\n")

# ---- register ----
gdb_csr()
