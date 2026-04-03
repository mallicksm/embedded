import gdb

def csr(name):
   try:
      return int(gdb.parse_and_eval(f"${name}"))
   except:
      return None

def pr(name, val):
   if val is None:
      print(f"{name:8} : <na>")
   else:
      print(f"{name:8} : 0x{val:08x}")

class DumpCSR(gdb.Command):
   def __init__(self):
      super().__init__("dumpcsr", gdb.COMMAND_USER)

   def invoke(self, arg, from_tty):
      print("\n=== CSR ===")

      for n in ["mstatus", "mepc", "mcause", "mtvec"]:
         pr(n, csr(n))

      print("============\n")

DumpCSR()
