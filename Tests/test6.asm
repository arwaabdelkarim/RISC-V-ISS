.text
.globl main

main:

addi s0,zero,10
xori t0,s0,40

Loop: beq t0,zero,exit
      slli s0,s0,1
      xori t0,s0,40
      sltiu a0,s0,40
      li a7,1
      ecall
      j  Loop
exit:

