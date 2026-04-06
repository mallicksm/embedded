import gdb

def csr(name):
   try:
      return int(gdb.parse_and_eval(f"${name}")) & 0xffffffff
   except:
      return None

def pr(name, val):
   if val is None:
      print(f"{name:10} : <na>")
   else:
      print(f"{name:10} : 0x{val:08x}")

class DumpCSR(gdb.Command):
   def __init__(self):
      super().__init__("dumpcsr", gdb.COMMAND_USER)

   def invoke(self, arg, from_tty):
      print("\n=== CSR: trap ===")
      for n in ["mepc", "mcause", "mtval", "mtvec"]:
         pr(n, csr(n))

      print("\n=== CSR: state ===")
      for n in ["mstatus", "misa"]:
         pr(n, csr(n))

      print("\n=== CSR: interrupt ===")
      for n in ["mie", "mip"]:
         pr(n, csr(n))

      print("\n=== CSR: misc ===")
      for n in ["mscratch"]:
         pr(n, csr(n))

      print("\n=================\n")

DumpCSR()
