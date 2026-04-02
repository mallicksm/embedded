set confirm off
set pagination off
set print pretty on
target remote :1234
set $pc = 0x80000000
set confirm on

define connect
  set confirm off
  set pagination off
  set print pretty on
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
