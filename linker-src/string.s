# CS 61C Summer 2016 Project 2-2 
# string.s

#==============================================================================
#                              Project 2-2 Part 1
#                               String README
#==============================================================================
# In this file you will be implementing some utilities for manipulating strings.
# The functions you need to implement are:
#  - strlen()
#  - strncpy()
#  - copy_of_str()
# Test cases are in linker-tests/test_string.s
#==============================================================================

.data
newline:	.asciiz "\n"
tab:	.asciiz "\t"

.text
#------------------------------------------------------------------------------
# function strlen()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string input
#
# Returns: the length of the string
#------------------------------------------------------------------------------
# C code
#
# int summ = 0;
# char *s = "hello";
# while (s != NULL) {
#		summ = summ + 1;
#		s++:
#	}
# return summ;

strlen:
	# YOUR CODE HERE
	add $v0, $0, $0
	beq $a0, $0, ret1

iterate:
	lb $t0, 0($a0)
	beq $t0, $0, ret1
	addi $a0, $a0, 1
	addi $v0, $v0, 1
	j iterate

ret1:
	jr $ra

#------------------------------------------------------------------------------
# function strncpy()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = pointer to destination array
#  $a1 = source string
#  $a2 = number of characters to copy
#
# Returns: the destination array
#------------------------------------------------------------------------------
# C code
#
# *x = $a0
# *y = $a1
# n = $a2
#
# int k = 0;
# for (k; k < n; k++) {
#   *x = * y;	
#	x++;	  	
#	y++;
# }
# return y;

strncpy:
	# YOUR CODE HERE
	addi   $t0, $0, 0
	addi   $t1, $a1, 0
	addi   $t2, $a0, 0
	slt    $t3, $t0, $a2
	beq    $t3, $0, ret

loop:
	lb $t4, 0($t1)
	sb $t4, 0($t2)
	addi $t1, $t1, 1
	addi $t2, $t2, 1
	addi $t0, $t0, 1
	slt $t3, $t0, $a2
	bne $t3, $0, loop

ret:
	addi $v0, $a0, 0
	jr $ra

#------------------------------------------------------------------------------
# function copy_of_str()
#------------------------------------------------------------------------------
# Creates a copy of a string. You will need to use sbrk (syscall 9) to allocate
# space for the string. strlen() and strncpy() will be helpful for this function.
# In MARS, to malloc memory use the sbrk syscall (syscall 9). See help for details.
#
# Arguments:
#   $a0 = string to copy
#
# Returns: pointer to the copy of the string
#------------------------------------------------------------------------------
# C code
#
# char *s = malloc(strlen(x) + 1);
# s = strncpy(s, x, strlen(x));
# return s;
#
#
copy_of_str:
	# YOUR CODE HERE
	# Prologue, storing $ra and $a0 and $s0 to the stack.
	addiu 	$sp, $sp, -12
	sw 		$ra, 0($sp)
	sw 		$a0, 4($sp)
	sw 		$s0, 8($sp)
	addi 	$s0, $a0, 0
	jal strlen				# Will contain the length of the string in $v0.

	addi 	$a1, $s0, 0
	addi 	$a2, $v0, 0		# Setting up the arguments required for a syscall that allocates memory for the copied string.

	addi 	$a0, $v0, 0
	addiu 	$v0, $0, 9			# Syscall for allocating space on the heap.
	syscall
	addi 	$a0, $v0, 0
	jal strncpy

	#Epilogue
	lw 		$s0, 8($sp)
	lw 		$a0, 4($sp)
	lw 		$ra, 0($sp)
	addiu 	$sp, $sp, -12
	jr 		$ra

###############################################################################
#                 DO NOT MODIFY ANYTHING BELOW THIS POINT                       
###############################################################################

#------------------------------------------------------------------------------
# function streq() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string 1
#  $a1 = string 2
#
# Returns: 0 if string 1 and string 2 are equal, -1 if they are not equal
#------------------------------------------------------------------------------
streq:
	beq $a0, $0, streq_false	# Begin streq()
	beq $a1, $0, streq_false
streq_loop:
	lb $t0, 0($a0)
	lb $t1, 0($a1)
	addiu $a0, $a0, 1
	addiu $a1, $a1, 1
	bne $t0, $t1, streq_false
	beq $t0, $0, streq_true
	j streq_loop
streq_true:
	li $v0, 0
	jr $ra
streq_false:
	li $v0, -1
	jr $ra			# End streq()

#------------------------------------------------------------------------------
# function dec_to_str() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Convert a number to its unsigned decimal integer string representation, eg.
# 35 => "35", 1024 => "1024". 
#
# Arguments:
#  $a0 = int to write
#  $a1 = character buffer to write into
#
# Returns: the number of digits written
#------------------------------------------------------------------------------
dec_to_str:
	li $t0, 10			# Begin dec_to_str()
	li $v0, 0
dec_to_str_largest_divisor:
	div $a0, $t0
	mflo $t1		# Quotient
	beq $t1, $0, dec_to_str_next
	mul $t0, $t0, 10
	j dec_to_str_largest_divisor
dec_to_str_next:
	mfhi $t2		# Remainder
dec_to_str_write:
	div $t0, $t0, 10	# Largest divisible amount
	div $t2, $t0
	mflo $t3		# extract digit to write
	addiu $t3, $t3, 48	# convert num -> ASCII
	sb $t3, 0($a1)
	addiu $a1, $a1, 1
	addiu $v0, $v0, 1
	mfhi $t2		# setup for next round
	bne $t2, $0, dec_to_str_write
	jr $ra			# End dec_to_str()
