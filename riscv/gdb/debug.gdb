set confirm off
set pagination off
set print pretty on
set print symbol off
set print array on
set print elements 50
set print repeats 10

target remote :1234
set $pc = 0x80000000

set confirm on

python
import os, sys
import importlib
if "gdb" not in sys.path:
   sys.path.append("gdb")
p = os.environ.get("PY_YAML_PATH")
if p not in sys.path:
    sys.path.insert(0,p)
import gdb_common
import gdb_printers
import gdb_ctx
import gdb_tasks
import gdb_csr
#
gdb_common   = importlib.reload(gdb_common)
gdb_printers = importlib.reload(gdb_printers)
gdb_ctx      = importlib.reload(gdb_ctx)
gdb_tasks    = importlib.reload(gdb_tasks)
gdb_csr      = importlib.reload(gdb_csr)
end

define connect
  set confirm off
  target remote :1234
  set $pc = 0x80000000
  set confirm on
end

set $tui_state = 1
define tt
  if $tui_state
    tui disable
    set $tui_state = 0
  else
    tui enable
    set $tui_state = 1
  end
end
