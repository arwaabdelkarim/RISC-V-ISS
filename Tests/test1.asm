.text

.globl main
main:
	auipc	sp,0x14
	mv	sp,sp
	li a0,14
	call fib

	li a7, 1
	ecall

	li a7, 10
	ecall
fib:
	li t0, 1
	bgt a0, t0, recurse
	ret
recurse:
	addi sp, sp, -12
	sw ra, 0(sp)
	sw a0, 4(sp)
	addi a0, a0, -1
	call fib
	sw a0, 8(sp)
	lw a0, 4(sp)
	addi a0, a0, -2
	call fib
	lw t0, 8(sp)
	add a0, a0, t0
	lw ra, 0(sp)
	addi sp, sp, 12
	ret

	
	
	
	
	
