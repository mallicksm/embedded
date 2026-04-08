import gdb
from gdb_common import val

# ---- command ----

class gdb_tasks(gdb.Command):
   def __init__(self):
      super().__init__("gdb_tasks", gdb.COMMAND_USER)

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

         print("[%d] %s state=%d sleep=%d" % (i, name, state, sleep))

         t = t["next"]
         i += 1

         if int(t) == 0 or t == start:
            break

      print("=================\n")

# ---- register ----

gdb_tasks()
