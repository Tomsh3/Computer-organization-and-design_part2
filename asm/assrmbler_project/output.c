#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "first _run.h"
#include "output.h"


enum OPCODE {
	add, sub, and, or , sll, sra, srl, beq, bne, blt, bgt, ble, bge, jal, lw, sw, reti, in, out, halt,
};

enum REGISTER {
	$zero, $imm, $v0, $a0, $a1, $t0, $t1, $t2, $t3, $s0, $s1, $s2, $gp, $sp, $fp, $ra,
};


//printRegisterNumber - print rd,rs,rt code into the memin file
// memin - pointer to a file to print in it , data - if it's a .world  will be flipped to zero // TODO
void printRegisterNumber(char *register_name, FILE *memin, int *data)
{ // basically a big if block that checks the name of the register and converts it to a number
	if (strcmp(register_name, "$zero") == 0)
		fprintf(memin, "%X", $zero);
	else if (strcmp(register_name, "$imm") == 0)
		fprintf(memin, "%X", $imm);
	else if (strcmp(register_name, "$v0") == 0)
		fprintf(memin, "%X", $v0);
	else if (strcmp(register_name, "$a0") == 0)
		fprintf(memin, "%X", $a0);
	else if (strcmp(register_name, "$a1") == 0)
		fprintf(memin, "%X", $a1);
	else if (strcmp(register_name, "$t0") == 0)
		fprintf(memin, "%X", $t0);
	else if (strcmp(register_name, "$t1") == 0)
		fprintf(memin, "%X", $t1);
	else if (strcmp(register_name, "$t2") == 0)
		fprintf(memin, "%X", $t2);
	else if (strcmp(register_name, "$t3") == 0)
		fprintf(memin, "%X", $t3);
	else if (strcmp(register_name, "$s0") == 0)
		fprintf(memin, "%X", $s0);
	else if (strcmp(register_name, "$s1") == 0)
		fprintf(memin, "%X", $s1);
	else if (strcmp(register_name, "$s2") == 0)
		fprintf(memin, "%X", $s2);
	else if (strcmp(register_name, "$gp") == 0)
		fprintf(memin, "%X", $gp);
	else if (strcmp(register_name, "$sp") == 0)
		fprintf(memin, "%X", $sp);
	else if (strcmp(register_name, "$fp") == 0)
		fprintf(memin, "%X", $fp);
	else if (strcmp(register_name, "$ra") == 0)
		fprintf(memin, "%X", $ra);
	else if (strcmp(register_name, "blank") == 0)
		*data = 0;
	else
		fprintf(memin, "0");
}

// printOpcode - print opcode code to datatofile  memin.txt , returns 1 if opcode was printed
// memin - pointer to a file to print in it
int printOpcode(char *opcode, FILE *memin)
{
	if (strcmp(opcode, "add") == 0)
		fprintf(memin, "%02X", add);
	else if (strcmp(opcode, "sub") == 0)
		fprintf(memin, "%02X", sub);
	else if (strcmp(opcode, "and") == 0)
		fprintf(memin, "%02X", and);
	else if (strcmp(opcode, "or") == 0)
		fprintf(memin, "%02X", or );
	else if (strcmp(opcode, "sll") == 0)
		fprintf(memin, "%02X", sll);
	else if (strcmp(opcode, "sra") == 0)
		fprintf(memin, "%02X", sra);
	else if (strcmp(opcode, "srl") == 0)
		fprintf(memin, "%02X", srl);
	else if (strcmp(opcode, "beq") == 0)
		fprintf(memin, "%02X", beq);
	else if (strcmp(opcode, "bne") == 0)
		fprintf(memin, "%02X", bne);
	else if (strcmp(opcode, "blt") == 0)
		fprintf(memin, "%02X", blt);
	else if (strcmp(opcode, "bgt") == 0)
		fprintf(memin, "%02X", bgt);
	else if (strcmp(opcode, "ble") == 0)
		fprintf(memin, "%02X", ble);
	else if (strcmp(opcode, "bge") == 0)
		fprintf(memin, "%02X", bge);
	else if (strcmp(opcode, "jal") == 0)
		fprintf(memin, "%02X", jal);
	else if (strcmp(opcode, "lw") == 0)
		fprintf(memin, "%02X", lw);
	else if (strcmp(opcode, "sw") == 0)
		fprintf(memin, "%02X", sw);
	else if (strcmp(opcode, "reti") == 0)
		fprintf(memin, "%02X", reti);
	else if (strcmp(opcode, "in") == 0)
		fprintf(memin, "%02X", in);
	else if (strcmp(opcode, "out") == 0)
		fprintf(memin, "%02X", out);
	else if (strcmp(opcode, "halt") == 0)
		fprintf(memin, "%02X", halt);
	else // not printed -its a word order
		return 0;
	return 1;
}

//gets memory stuct (contain head to memory list) and output file memin. prints the memory into the file
void print_data_file(Memory* mem, FILE *memin)
{
	int memory_pos = 0;//memory_pos- hold the line current position ,
	int data = 0;    // data - word for .word
	
		while (mem->head != NULL && memory_pos <= mem->last) {
			Memory_Line_list *currentLine = get_line_by_pos(mem->head, memory_pos); // get the current line's data from memory list. 
			int flag_opcode_printed = 1;

			///// now we Printing output. 
			if (currentLine == NULL) { // if data of the memory_pos line does not exist its a emprty/void pc address print zeroes
				fprintf(memin, "00");  // if no opcode print 2 zeros
				fprintf(memin, "000"); // Printing Rd  Rs   Rt  registers
				fprintf(memin, "%03X", 0 & 0xfff); //print zero to immediate on a null line. 
			}

			if (currentLine != NULL) {
				// print opcode or word format
				flag_opcode_printed = printOpcode(currentLine->opcode, memin); //if opcode exist print it and flag=1 was if exist dont print and flag=0

				if (flag_opcode_printed == 0 && (strcmp(currentLine->opcode, "blank") == 0)) {// if there is no opcode. this is .word command
					data = string_to_int(currentLine->imm);
					fprintf(memin, "%08X", data);  //Print immidiate in hex format
				}
				//    print registers    printRegisterNumber know to hendle .word command
				printRegisterNumber(currentLine->rd, memin, &data);
				printRegisterNumber(currentLine->rs, memin, &data);
				printRegisterNumber(currentLine->rt, memin, &data);

				// check wheter to print the immediate and print it in format
				if (strcmp(currentLine->opcode, "blank") != 0)  // its not .word command
				{
					data = string_to_int(currentLine->imm);
					fprintf(memin, "%03X", data & 0xfff);  //Print immidiate in hex. the & 0xfff is format to handle data in hexadecimal also negative numbers
				}
			}
			if (memory_pos != mem->last)//Print \n in the end of the line
				fprintf(memin, "\n");
			memory_pos++;  //advance to the next line pc
		}
}

