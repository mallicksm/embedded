.section .text
.globl thread_switch
.type thread_switch, @function

#------------------------------------------------------------------------------
# thread_switch(old, new)
#
# a0 = old context (may be NULL → skip save)
# a1 = new context (must be valid)
#
# What it does:
#   - Saves callee-saved registers of the current thread into *old
#   - Restores callee-saved registers from *new
#   - Switches execution to the new thread
#
# Register policy:
#   - Saves: s0–s11, ra, sp   (callee-saved + control state)
#   - Does NOT touch: a*, t*  (caller-saved handled by trap)
#
# Control flow:
#   - If old != NULL → save current context
#   - Always restore new context
#   - ret → resumes execution in new thread
#------------------------------------------------------------------------------

thread_switch:
   beqz a0, 2f

   # --------------------------------------------------
   # Save current thread context (callee-saved)
   # --------------------------------------------------
   sw   s0,  0(a0)
   sw   s1,  4(a0)
   sw   s2,  8(a0)
   sw   s3, 12(a0)
   sw   s4, 16(a0)
   sw   s5, 20(a0)
   sw   s6, 24(a0)
   sw   s7, 28(a0)
   sw   s8, 32(a0)
   sw   s9, 36(a0)
   sw  s10, 40(a0)
   sw  s11, 44(a0)

   sw   ra, 48(a0)
   sw   sp, 52(a0)

2:
   # --------------------------------------------------
   # Restore next thread context (callee-saved)
   # --------------------------------------------------
   lw   s0,  0(a1)
   lw   s1,  4(a1)
   lw   s2,  8(a1)
   lw   s3, 12(a1)
   lw   s4, 16(a1)
   lw   s5, 20(a1)
   lw   s6, 24(a1)
   lw   s7, 28(a1)
   lw   s8, 32(a1)
   lw   s9, 36(a1)
   lw  s10, 40(a1)
   lw  s11, 44(a1)

   lw   ra, 48(a1)
   lw   sp, 52(a1)

   # --------------------------------------------------
   # Return into new thread
   # --------------------------------------------------
   ret

.size thread_switch, .-thread_switch
