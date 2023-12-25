#ifndef second_run_H
#define second_run_H

#include <stdio.h>






/*****************************************************************
						declaration
*****************************************************************/

// this struct will be used to save the memory lines
typedef struct Memory_Line_list {
	char opcode[6];// opcode name can be 5 characters and '/0' to  8 bit 
	char rd[6];    //  reg name can be 5 characters and '/0' to a 4 bit 
	char rs[6];    //  reg name can be 5 characters and '/0' to a 4 bit 
	char rt[6];    //  reg name can be 5 characters and '/0' to a 4 bit 
	char imm[51];  //  immediate value or label name can be 50 characters and '/0' to 12 bit 
	int pos_pc;    // position of character in line
	struct Memory_Line_list* next;
}Memory_Line_list;


// Memory struct and related functions.
//it is used so the --second iteration-- can return two values.
typedef struct Memory {
	Memory_Line_list* head;// head of memory line list
	int last;// the position of the last line (pc) in the memory
}Memory;



Memory* SecondRun(FILE* assembly);
void free_memory_struct(Memory* mem);
int string_to_int(char* string);
Memory_Line_list* get_line_by_pos(Memory_Line_list* list, int pos);



#endif
