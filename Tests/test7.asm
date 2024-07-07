.text 
.globl main

main: 

addi t0,zero,40
addi t1,zero,11

Loop: bltu t0,t1,Exit
      srli t0,t0,1
      j    Loop
Exit:

