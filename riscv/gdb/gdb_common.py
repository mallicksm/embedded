import gdb

# ---- helpers ----
def load_yaml(path):
   try:
      import yaml
      with open(path) as f:
         return yaml.safe_load(f)
   except:
      return {}
def get_bits(val, spec):
   try:
      if isinstance(spec, int):
         return (val >> spec) & 1

      if isinstance(spec, str):
         spec = spec.strip()

         # ---- range ----
         if ":" in spec:
            hi, lo = spec.split(":")
            hi = int(hi.strip())
            lo = int(lo.strip())
            mask = (1 << (hi - lo + 1)) - 1
            return (val >> lo) & mask

         # ---- single bit (string) ----
         bit = int(spec)
         return (val >> bit) & 1

      return 0
   except:
      return 0
def print_reg_yaml(db, name, val):
   if val is None:
      print("%-10s : <na>" % name)
      return

   print("%-10s : 0x%08x" % (name, val))

   if name not in db:
      return

   reg = db[name]
   fields = reg.get("fields", [])

   for f in fields:
      try:
         fname = f.get("name", "?")
         bits  = f.get("bits", 0)
         desc  = f.get("desc", "")

         v = get_bits(val, bits)

         # ---- compute width (FIXED) ----
         if isinstance(bits, int):
            width = 1
         elif isinstance(bits, str):
            bits = bits.strip()

            if ":" in bits:
               hi, lo = bits.split(":")
               hi = int(hi.strip())
               lo = int(lo.strip())
               width = hi - lo + 1
            else:
               width = 1   # single-bit string
         else:
            width = 1

         # ---- hex width ----
         hex_width = (width + 3) // 4
         fmt = "%%0%dX" % hex_width
         val_str = fmt % v

         print("   %-6s     : %d'h%s %s" % (fname, width, val_str, desc))

      except:
         pass

def xprint_reg_yaml(db, name, val):
   if val is None:
      print("%-10s : <na>" % name)
      return

   print("%-10s : 0x%08x" % (name, val))

   if name not in db:
      return

   reg = db[name]
   fields = reg.get("fields", [])

   for f in fields:
      try:
         fname = f.get("name", "?")
         bits  = f.get("bits", 0)
         desc  = f.get("desc", "")

         v = get_bits(val, bits)

         # ---- compute width ----
         if isinstance(bits, int):
            width = 1
         else:
            hi, lo = bits.split(":")
            hi = int(hi.strip())
            lo = int(lo.strip())
            width = hi - lo + 1

         # ---- hex digit width ----
         hex_width = (width + 3) // 4   # bits → hex digits

         # ---- format value ----
         fmt = "%%0%dX" % hex_width
         val_str = fmt % v

         # ---- print Verilog style ----
         print("   %-6s : %d'h%s %s" % (fname, width, val_str, desc))

      except:
         pass

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
