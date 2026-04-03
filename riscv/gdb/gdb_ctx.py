import gdb

# ---- helpers ----

def val(expr):
   try:
      return gdb.parse_and_eval(expr)
   except:
      return None

def hx(v):
   try:
      return int(v)
   except:
      return 0

def pr(name, v):
   if v is None:
      print(f"{name:6}: <na>")
   else:
      print(f"{name:6}: 0x{hx(v):08x}")

# ---- dump current context ----

class DumpCtx(gdb.Command):
   def __init__(self):
      super().__init__("dumpctx", gdb.COMMAND_USER)

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

# ---- dump all tasks ----
class DumpTasks(gdb.Command):
   def __init__(self):
      super().__init__("dumptasks", gdb.COMMAND_USER)

   def invoke(self, arg, from_tty):
      print("\n=== TASK LIST ===")

      t = val("g_first_task")
      if t is None or int(t) == 0:
         print("no tasks")
         return

      start = t
      i = 0

      while True:
         try:
            name = t["name"].string()
         except:
            name = "<noname>"

         try:
            state = int(t["state"])
         except:
            state = -1

         try:
            sleep = int(t["sleep_ticks"])
         except:
            sleep = -1

         print(f"[{i}] {name} state={state} sleep={sleep}")

         t = t["next"]
         i += 1

         if int(t) == 0 or t == start:
            break

      print("=================\n")

DumpCtx()
DumpTasks()
