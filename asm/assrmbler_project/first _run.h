#ifndef first_run_H
#define first_run_H

#include <stdio.h>
#include "second_run.h"


/*****************************************************************
						Globals const
*****************************************************************/


#define  MAX_LINE 301// The maximum line size of the input file is 300.
#define  MAX_LABEL_LEN 51 // the maximum lable size is 50.
#define  MAX_MEMO_LINES 512



/*****************************************************************
						declaration
*****************************************************************/

// Label struct

// the label linked list will help us store info about the labels. it will be built at the first iteration and used and the second.
typedef struct Label {
	char name[MAX_LABEL_LEN];// name will store the name of the label
	int address; // address will store the line number (pc) and will be the immediate value in related jump and branch commands
	struct Label* next;
} label;



void clean_comment(char* line);
void clean_line(char* line);
void clean_white_spaces(char* line);
label* first_run(FILE *assembly);
void free_label_list(label* head);
int find_label(label* label_list, char* label_to_find);
void LableChange(Memory_Line_list* memory_list, label* label_list);







#endif