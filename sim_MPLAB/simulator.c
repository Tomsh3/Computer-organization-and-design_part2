#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "btn.h"
#include "swt.h"

#define MAX_LINE 8 // maximum line length in memin.txt.
#define MAX_LABEL 50 // the maximum lable size is 50 and .
#define MAX_MEMO_LINES 512 // maximum number of lines in the memory.
#define NUM_OF_REG 16 // number of registers.
#define NUM_OF_IOREG 14 // number of IO registers.


typedef struct Format
{
	int opcode; // 8 bits
	int rd; // 4 bits
	int rs; // 4 bits
	int rt; // 4 bits
	int imm; // 12 bits.
}Format;

// Variables Definitions:
int PC = 0; // program counter
int counter_cycles = 0; // counts the number of cycles
int register_arr[NUM_OF_REG] = { 0 }; //  16 registers 
int IO_register_arr[NUM_OF_IOREG] = { 0 }; // 14 IO registers
int memory[MAX_MEMO_LINES]; // initiate memory array
char memin_arr[MAX_MEMO_LINES][MAX_LINE]; // initiate memin array


Format inst = { 0 }; // instruction 
Format* ptr_inst = &inst; // instruction's address

// Functions Decleration:
void build_instruction(int PC, Format* ptr_inst);
int execute(Format* ptr_inst);
void pause_check(int BTNU);


int main(int argc, char* argv[])
{
	int PC = 0; // restart PC
	int counter_cycles = 0; // restart counter
	int test_execution = 0; //Jump Indicator
	int BTNU = 0;

	if (SWT_GetValue(7) == 0) // if SW7 is off -> load "fib" memin
	{
		int memory[MAX_MEMO_LINES] = { 0x00D01001, 0x04DD1007, 0x0E301020, 0x0D100006, 0x0F201021, 0x13000000, 0x00DD1FFD, 0x0F9D1002, 0x0FFD1001, 0x0F3D1000, 0x00501001, 0x0A13500E, 0x00230000, 0x07100018, 0x01331001, 0x0D100006, 0x00921000, 0x0E3D1000, 0x01331002, 0x0D100006, 0x00229000, 0x0E3D1000, 0x0EFD1001, 0x0E9D1002, 0x00DD1003, 0x07F00000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000007 }; // memory size
		char memin_arr[MAX_MEMO_LINES][MAX_LINE] = { "00D01001", "04DD1007", "0E301020", "0D100006", "0F201021", "13000000", "00DD1FFD", "0F9D1002", "0FFD1001", "0F3D1000", "00501001", "0A13500E", "00230000", "07100018", "01331001", "0D100006", "00921000", "0E3D1000", "01331002", "0D100006", "00229000", "0E3D1000", "0EFD1001", "0E9D1002", "00DD1003", "07F00000", "00000000", "00000000", "00000000", "00000000", "00000000", "00000000", "00000007" };
	}

	else // if SW7 is on -> load "stoper" memin
	{
		/// TODO - memin of STOPER assembly
	}



	while (PC >= 0 && PC < MAX_MEMO_LINES)
	{
		pause_check(BTNU);
		build_instruction(PC, ptr_inst);
		test_execution = execute(ptr_inst);
		counter_cycles++;
		if (test_execution)
			continue; // the execution did a jump to  another place so we dont need to promote PC
		PC++;
	}
	
	return 0;
}


// This function executes "PAUSE"
void pause_check(int BTNU)
{
	if (BTN_GetValue('L') == 1) // check if BTNL was pressed, if so - PAUSE
	{
		BTNU = 1;
		while (BTNU == 1) // when BTNL is pressed again - exit loop and return to main loop
		{
			if (BTN_GetValue('R') == 1) // if BTNR was pressed - continue to next PC, but BTNL remains "1" - meaning the main while loop will pause in the next iteration
			{
				break;
			}
			if (BTN_GetValue('L') == 1) // if BTNL was pressed again - resume to main loop (stop "PAUSE" process) 
			{
				BTNU = 0;
			}
			continue;
		}
	}
}

// This function builds the components of the commands (op,rt,rd,...) 
void build_instruction(int PC, Format* ptr_inst)
{
	int num;
	char op_code[3] = { memin_arr[PC][0],memin_arr[PC][1],'\0' }; // op_code will get 2 hexa digits
	ptr_inst->opcode = strtoul(op_code, NULL, 16); // convert the opcode to decimal number

	char rd[2] = { memin_arr[PC][2],'\0' };// rd will get 2 hexa digits
	ptr_inst->rd = strtoul(rd, NULL, 16); // convert the rd to decimal number

	char rs[2] = { memin_arr[PC][3],'\0' };// rs will get 2 hexa digits
	ptr_inst->rs = strtoul(rs, NULL, 16); // convert the rs to decimal number

	char rt[2] = { memin_arr[PC][4],'\0' };// rt will get 2 hexa digits
	ptr_inst->rt = strtoul(rt, NULL, 16); // convert the rt to decimal number

	char imm[4] = { memin_arr[PC][5],memin_arr[PC][6],memin_arr[PC][7],'\0' };// imm will get 3 hexa digits
	ptr_inst->imm = strtoul(imm, NULL, 16); // convert the imm to decimal number

	// check if rd / rs / rt is imm, if yes - load the imm to the Reg Array
	if (ptr_inst->rs == 1) { // rs is immidiate
		num = strtoul(imm, NULL, 16); // convert the imm to decimal and sign extend it
		/*if (imm[0] == 'F') { // check if imm is a negative number
			num |= 0xfffff000;
		}*/
		ptr_inst->imm = num;
		register_arr[ptr_inst->rs] = ptr_inst->imm;
	}

	else if (ptr_inst->rt == 1) { // rt is immidiate
		num = strtoul(imm, NULL, 16); // convert the imm to decimal and sign extend it
		/*if (imm[0] == 'F') { // check if imm is a negative number
			num |= 0xfffff000;
		}*/
		ptr_inst->imm = num;
		register_arr[ptr_inst->rt] = ptr_inst->imm;
	}
	else if (ptr_inst->rd == 1) { // rd is immidiate
		num = strtoul(imm, NULL, 16); // convert the imm to decimal and sign extend it
		/*if (imm[0] == 'F') { // check if imm is a negative number
			num |= 0xfffff000;
		}*/
		ptr_inst->imm = num;
		register_arr[ptr_inst->rd] = ptr_inst->imm;
	}
	else {//the instruction did not use imm, reset imm to 0
		register_arr[1] = 0;
	}
}

// This function identifies the opdode and executes the instruction (registers, memory, PC...) 
int execute(Format* ptr_inst)
{
	switch (ptr_inst->opcode)
	{
	case 0x0: // add
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt];
		return 0;

	case 0x1: //sub
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] - register_arr[ptr_inst->rt];
		return 0;

	case 0x2: //and
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] & register_arr[ptr_inst->rt];
		return 0;

	case 0x3: //or
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] | register_arr[ptr_inst->rt];
		return 0;

	case 0x4: //sll
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] << register_arr[ptr_inst->rt];
		return 0;

	case 0x5: //sra
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] >> register_arr[ptr_inst->rt];
		return 0;

	case 0x6: //srl
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] >> register_arr[ptr_inst->rt];
		register_arr[ptr_inst->rd] = 0x7 && register_arr[ptr_inst->rd];
		return 0;

	case 0x7: //beq
		if (register_arr[ptr_inst->rs] == register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0x8: //bne 
		if (register_arr[ptr_inst->rs] != register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0x9: //blt
		if (register_arr[ptr_inst->rs] < register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0xA: //bgt
		if (register_arr[ptr_inst->rs] > register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0xB: //ble
		if (register_arr[ptr_inst->rs] <= register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0xC: //bge
		if (register_arr[ptr_inst->rs] >= register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0xD: //jal
		register_arr[15] = PC + 1;
		PC = register_arr[ptr_inst->rd];
		return 1;

	case 0xE: //lw
		register_arr[ptr_inst->rd] = memory[(register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt]) % MAX_MEMO_LINES]; //considering the size limitation
		return 0;

	case 0xF: //sw
		memory[(register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt]) % MAX_MEMO_LINES] = register_arr[ptr_inst->rd];
		return 0;

	case 0x13: //halt
		PC = -2;//will stop the loop after the itaration
		return 1;
	
	case 0x10: //reti
		PC = IO_register_arr[7];// Jump to the value stored in IOregister[7] (PC of interrupt return address)
		return 1;

	case 0x11: //in
		register_arr[ptr_inst->rd] = IO_register_arr[register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt]]; // Copy the value in IOregister[ R[rs] + R[rt] ] into R[rd]
		return 0;

	case 0x12: //out
		IO_register_arr[register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt]] = register_arr[ptr_inst->rd]; // Copy the value in R[rd] into IOregister[ R[rs] + R[rt] ] 
		return 0;
	}
}
