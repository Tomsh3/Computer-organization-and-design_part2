	add $sp, $zero, $imm, 1		# set $sp = 1
	sll $sp, $sp, $imm, 7		# set $sp = 1 << 7 = 128
	lw $a0, $zero, $imm, 32		# get x from address 32
	jal     $imm, 	$zero,    $zero, fib		# calc $v0 = fib(x)
	sw $v0, $zero, $imm, 33		# store fib(x) in 33
	halt $zero, $zero, $zero, 0	# halt

	# fib function
fib:	add $sp, $sp, $imm, -3		# adjust stack for 3 items
	sw $s0, $sp, $imm, 2		# save $s0
	sw $ra,$sp,$imm,1# save return address
	sw $a0, $sp, $imm, 0		# save argument
add $t0, $zero, $imm, 1		# $t0 = 1
	bgt $imm, $a0, $t0, L1		# jump to L1 if x > 1
	add $v0, $a0, $zero, 0		# otherwise, fib(x) = x, copy input
	beq $imm, $zero, $zero, L2	# jump to L2
L1:
	sub $a0, $a0, $imm, 1		# calculate x - 1
	jal $imm, $zero, $zero, fib	# calc $v0=fib(x-1)
	add $s0, $v0, $imm, 0		# $s0 = fib(x-1)
	lw $a0, $sp, $imm, 0		# restore $a0 = x
	sub $a0, $a0, $imm, 2		# calculate x - 2
	jal $imm, $zero, $zero, fib	# calc fib(x-2)
	add $v0, $v0, $s0, 0		# $v0 = fib(x-2) + fib(x-1)
	lw $a0, $sp, $imm, 0		# restore $a0
	lw $ra, $sp, $imm, 1		# restore $ra
	lw $s0, $sp, $imm, 2		# restore $s0
L2:
	add $sp, $sp, $imm, 3		# pop 3 items from stack
	beq $ra, $zero, $zero, 0	# and return
	.word 32 7
