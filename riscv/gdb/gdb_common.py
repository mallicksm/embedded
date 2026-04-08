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
