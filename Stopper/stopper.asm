
		# initialize IOregisters:

		add $t2, $zero, $imm, 1023						 # t2 = 1023, the max timer value
		out $t2, $zero, $imm, 13						 # export 1023 to timermax IOregister
		add $t2, $zero, $imm, 1							 # $t2 = 1 
		out $t2, $zero, $imm, 11						 # export the value 1 to the timerenable IOregister

		# initialize stopper digits:

		add  $s0, $zero, $imm, 0						 # $s0 contains the current clock 1st digit (starts from 1 since the first IRQ0 is after 1 second)
		add  $s1, $zero, $zero, 0						 # $s1 contains the current clock 2nd digit
		add  $s2, $zero, $zero, 0						 # $s2 contains the current clock 3rd digit 
		add  $a0, $zero, $zero, 0						 # $a0 contains the current clock 4th digit
		add  $a1, $zero, $zero, 0						 # $a1 contains the current clock hexa value, including all the 4 digits

							   
IRQ CHECK 0:

		in $t0, $zero, $imm, 3							 # check if irq0status IOregister is ON
		beq $imm, $zero, $t0, IRQ CHECK 1				 # if $t0 == 0 then jump to IRQ CHECK 1, a second didn't pass yet
		add $t0, $zero, $imm, IRQ0						 # $t0 = the PC of IRQ0
		out $t0, $zero, $imm, 6							 # export IRQ0 PC to irqhandler IOregister	
		out $imm, $zero, $zero, 1						 # a second passed - change irq0enable IOregister to 1
					

IRQ CHECK 1:

		in $t0, $zero, $imm, 4							 # check if irq1status IOregister is ON
		beq $imm, $zero, $t0, IRQ CHECK 2				 # if $t0 == 0 then jump to IRQ CHECK 2, BTNC hasn't been pressed yet
		add $t0, $zero, $imm, IRQ1						 # $t0 = the PC of IRQ1
		out $t0, $zero, $imm, 6							 # export IRQ0 PC to irqhandler IOregister
		out $imm, $zero, $imm, 1						 # change irq1enable IOregister to 1


IRQ CHECK 2:

		in $t0, $zero, $imm, 5						 	 # check if irq2status IOregister is ON
		beq $imm, $zero, $t0, IRQ CHECK 0				 # if $t0 == 0 then jump to IRQ CHECK 0, BTND hasn't been pressed yet
		add $t0, $zero, $imm, IRQ2						 # $t0 = the PC of IRQ2
		out $t0, $zero, $imm, 6							 # export IRQ0 PC to irqhandler IOregister
		add $t0, $zero, $imm, 1							 # $t0 = 1					
		out $t0, $zero, $imm, 2						     # change irq2enable IOregister to 1
		beq $imm, $zero, $zero, IRQ CHECK 0				 # return to IRQ check routine

IRQ0:
		
		add $t2 , $zero , $imm, 9			             # $t2 = 9 -> used to check 1st digit overflow
		beq $imm , $t2 , $s0, INCREMENT SECOND DIGIT	 # check if the 1st digit is about to overflow (bigger than 9), if so - jump to inner loop
		add $s0 , $s0 , $imm, 1							 # increment 1st digit: $s0 = $s0 + 1
		beq $imm, $zero, $zero, SEGMENTS UPDATE			 # jump to SEGMENTS UPDATE after incrementing


SEGMENTS UPDATE:

		add $a1, $zero, $a0, 0							 # add the 4th digit to $a1
		sll $a1, $a1, $imm , 4							 # shift the digits one hexa-digit left
		add $a1, $a1, $s2, 0							 # add the 3rd digit to $a1 
		sll $a1, $a1, $imm , 4							 # shift the digits one hexa-digit left
		add $a1, $a1, $s1, 0							 # add the 2nd digit to $a1
		sll $a1, $a1, $imm , 4							 # shift the digits one hexa-digit left
		add $a1, $a1, $s0, 0							 # add the 1st digit to $a1

		# now $a1 contains the current time in the following format: 0xMmSs (M = 10*minute, m = minutes, S = 10*second, s = seconds)

		out $a1, $zero, $imm, 10						 # display current clock value by exporting it to display7seg IOregister

		# return:

		out $zero, $zero, $imm, 3						 # change irq0status IOregister to OFF 
		out $zero, $zero, $zero, 0						 # change irq0enable IOregister to OFF 
		reti $zero, $zero, $zero, 0						 # no overflow, return to IRQ CHECK


INCREMENT SECOND DIGIT:

		add $t2 , $zero , $imm, 5			             # $t2 = 5 -> used to check 2nd digit overflow
		beq $imm , $t2 , $s1, INCREMENT THIRD DIGIT		 # check if the 2nd digit is about to overflow (bigger than 5), if so - jump to inner loop
		add $s1 , $s1 , $imm, 1							 # increment 2nd digit: $s1 = $s1 + 1
		add $s0 , $zero , $imm, 0						 # reset 1st digit: $s0 = 0 
		beq $imm, $zero, $zero, SEGMENTS UPDATE			 # jump to SEGMENTS UPDATE after incrementing



INCREMENT THIRD DIGIT:
		
		add $t2 , $zero , $imm, 9			             # $t2 = 9 -> used to check 3rd digit overflow
		beq $imm , $t2 , $s2, INCREMENT FOURTH DIGIT	 # check if the 3th digit is about to overflow (bigger than 9), if so - jump to inner loop
		add $s2 , $s2 , $imm, 1							 # increment 3rd digit: $s2 = $s2 + 1
		add $s0 , $zero , $imm, 0						 # reset 1st digit: $s0 = 0 
		add $s1 , $zero , $imm, 0						 # reset 2nd digit: $s1 = 0
		beq $imm, $zero, $zero, SEGMENTS UPDATE			 # jump to SEGMENTS UPDATE after incrementing


INCREMENT FOURTH DIGIT:

		add $t2 , $zero , $imm, 5			             # $t2 = 5 -> used to check 4th digit overflow
		beq $imm , $t2 , $a0, RESET IRQ0                 # check if the 4th digit is about to overflow (bigger than 5), if so - jump to RESET
		add $a0 , $a0 , $imm, 1							 # increment 4th digit: $a0 = $a0 + 1
		add $s0 , $zero , $imm, 0						 # reset 1st digit: $s0 = 0 
		add $s1 , $zero , $imm, 0						 # reset 2nd digit: $s1 = 0
		add $s2 , $zero , $imm, 0						 # reset 3rd digit: $s2 = 0
		beq $imm, $zero, $zero, SEGMENTS UPDATE			 # jump to SEGMENTS UPDATE after incrementing


RESET IRQ0:

		out $zero, $zero, $imm, 10						 # display 0 by changine display7seg IOregister to 0
		add  $s0, $zero, $imm, 0						 # $s0 contains the current clock 1st digit (starts from 1 since the first IRQ0 is after 1 second)
		add  $s1, $zero, $zero, 0						 # $s1 contains the current clock 2nd digit
		add  $s2, $zero, $zero, 0						 # $s2 contains the current clock 3rd digit 
		add  $a0, $zero, $zero, 0						 # $a0 contains the current clock 4th digit
		add  $a1, $zero, $zero, 0						 # $a1 contains the current clock hexa value, including all the 4 digits
		out $zero, $zero, $imm, 3						 # change irq0status IOregister to OFF 
		reti $zero, $zero, $zero, 0						 # no overflow, return to IRQ CHECK

IRQ1:
		
		out $zero, $zero, $imm, 4						 # change irq1status IOregister to 0
		
IRQ1_EXECUTE:

		in $t0, $zero, $imm, 4							 # check if irq1status IOregister is 1, meaning BTNC has been clicked again
		beq $imm, $zero, $t0, IRQ1_EXECUTE				 # if $t0 == 0 then BTNC hasn't been clicked yet, resart loop

		# return:

		out $zero, $zero, $imm, 4					 	 # turn irq1status IOregister to OFF
		out $zero, $zero, $imm, 1						 # turn irq1enable IOregister to OFF
		reti $zero, $zero, $zero, 0						 # BTNC has been clicked, return to the PC that was saveed before handling IRQ1
		

IRQ2:
		
		# perform reset:

		out $zero, $zero, $imm, 5						 # change irq2status IOregister to 0
		out $zero, $zero, $imm, 10						 # display 0 by changine display7seg IOregister to 0
		add  $s0, $zero, $imm, 0						 # $s0 contains the current clock 1st digit (starts from 1 since the first IRQ0 is after 1 second)
		add  $s1, $zero, $zero, 0						 # $s1 contains the current clock 2nd digit
		add  $s2, $zero, $zero, 0						 # $s2 contains the current clock 3rd digit 
		add  $a0, $zero, $zero, 0						 # $a0 contains the current clock 4th digit
		add  $a1, $zero, $zero, 0						 # $a1 contains the current clock hexa value, including all the 4 digits 

		# return:

		out $zero, $zero, $imm, 5						 # turn irq2status IOregister to OFF
		out $zero, $zero, $imm, 2						 # turn irq2enable IOregister to OFF
		reti $zero, $zero, $zero, 0						 # the reset has been done, return to the PC that was saveed before handling IRQ2



	