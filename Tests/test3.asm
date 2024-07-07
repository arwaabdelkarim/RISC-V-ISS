.text 
.globl main

main:

li   s0, 0x200
li   s2, -4
li   s5,-88
addi t0,zero,5
add  t1,zero,zero
add  t2,zero,zero

sb   s2,0(s0)
sh   s5,4(s0)
lb   t3,0(s0)
lh   t4,4(s0)
lbu t5, 0(s0)
lhu t6, 4(s0)


add a0,zero,t3
li a7,1
ecall
	
add a0,zero,t4
li a7,1
ecall

mv a0, t5
li a7,1
ecall	

mv a0, t6
li a7,1
ecall	

li a7, 10
ecall

