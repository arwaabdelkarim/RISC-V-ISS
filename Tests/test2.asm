#this program was implemented using RARS assembler 
.text
.globl main

main: 
	auipc	sp,0x10
	mv	sp,sp
	li a0,10
	call add

	li a7, 1
	ecall

	li a7, 10
	ecall

add:	
	bne a0, zero, recurse
	ret

recurse:
	addi sp, sp, -12 #allocate stack
	sw ra, 4(sp)    #store ra in stack
	sw a0, 0(sp)   #store n in stack 
	
	addi a0, a0, -1 
	call add
	sw a0, 8(sp)
	
	lw t0, 8(sp)
	lw a0, 0(sp)
	add a0,a0,t0
	lw ra, 4(sp)  #load return address
	addi sp, sp, 12 #deallocate stack 
	ret
	



