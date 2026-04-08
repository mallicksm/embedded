import gdb
from gdb_common import val, hx

# ---- helpers ----

def pr(name, v):
   if v is None:
      print(f"{name:6}: <na>")
   else:
      print(f"{name:6}: 0x{hx(v):08x}")

# ---- command ----

class gdb_ctx(gdb.Command):
   def __init__(self):
      super().__init__("gdb_ctx", gdb.COMMAND_USER)

   def invoke(self, arg, from_tty):
      print("\n=== CTX ===")

      ctx = val("g_current_task->ctx")
      if ctx is None:
         print("no current task")
         return

      regs = [
         "ra", "sp",
         "s0","s1","s2","s3","s4","s5",
         "s6","s7","s8","s9","s10","s11"
      ]

      for r in regs:
         try:
            pr(r, ctx[r])
         except:
            print(f"{r:6}: <err>")

      print("===========\n")

# ---- register ----

gdb_ctx()
