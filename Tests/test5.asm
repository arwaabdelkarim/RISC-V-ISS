.text
.global main

main:
	addi x18, x18, 17
	addi x19, x19, 29
	slt x5, x18, x19
	mv x10, x5
	li a7, 1
	ecall
	li x6, 2
	sra x18, x18, x6
	slti x10, x18, 18
	li a7, 1
	ecall
	addi x18, x18, 17
	addi x19, x19, 17
	xor x10, x18, x19
	li a7,1 
	ecall
	sltu x10, x19, x18
	li a7,1 
	ecall
	andi x10, x18, 29
	li a7,1
	ecall

