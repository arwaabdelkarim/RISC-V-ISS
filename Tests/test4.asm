.text
.globl main

main:
	
	addi a0,zero,5 # stores the multiplier
	add s0,a0,zero
	
	addi t2,zero,25
	or   a0,zero,t2
	add s1,a0,zero # stores the multiplicand
	
	add s2,zero,zero # stores the value of the multiplicatiom
	addi t0,zero,0x001
	
Loop:   beq s0,zero,Exit
	andi t1,s0,0x001
	ori t3,zero,1
	srl  s0,s0,t3 
	bne  t1,t0,left
	add  s2,s2,s1
left:	sll s1,s1,t3
	  j Loop
Exit:	
	add a0,s2,zero
	li a7,1
	ecall	
	

