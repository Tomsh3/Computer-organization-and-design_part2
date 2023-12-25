#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "first _run.h"
#include "second_run.h"





// creates new memory line
Memory_Line_list* create_memory_line(char* opcode, char* rd, char* rs, char* rt, char* imm, int current_pos) {
	Memory_Line_list* new_line = (Memory_Line_list*)malloc(sizeof(Memory_Line_list));
	if (new_line != NULL) {	// allocation successful init struct
		strcpy(new_line->opcode, opcode);
		strcpy(new_line->rd, rd);
		strcpy(new_line->rs, rs);
		strcpy(new_line->rt, rt);
		strcpy(new_line->imm, imm);
		new_line->pos_pc = current_pos;
		new_line->next = NULL;
	}
	return new_line;
}



// adds line to the end of the memory list
Memory_Line_list* add_line_to_memory_list(Memory_Line_list* head, char* opcode, char* rd, char* rs, char* rt, char* imm, int address_pos)
{
	Memory_Line_list* tail = head;;	// hold the last line of the list

	Memory_Line_list* new_instruction_line = create_memory_line(opcode, rd, rs, rt, imm, address_pos);
	if (new_instruction_line == NULL)	// if the new line is null. do nothing
		return NULL;

	if (head == NULL)// for empty list
		return new_instruction_line;

	while (tail->next != NULL)// go to the tail
		tail = tail->next;
	tail->next = new_instruction_line;// add the new line
	return head;
}


// find line by his position
// return pointer to the line or null if not found
Memory_Line_list* get_line_by_pos(Memory_Line_list* list, int pos) {
	while (list != NULL && list->pos_pc != pos)
		list = list->next;
	return list;
}


//create memory structure
Memory* create_mem(Memory_Line_list* head, int pos1)
{
	Memory* mem = (Memory*)malloc(sizeof(Memory));
	mem->head = head;
	mem->last = pos1;
	return(mem);
}

// free memory line list
void free_memory_Line_list(Memory_Line_list* head) {

	Memory_Line_list* temp;
	while (head != NULL)
	{
		temp = head;
		head = head->next;
		free(temp);
	}
}

//free memory struct 
void free_memory_struct(Memory* mem) {
	free_memory_Line_list(mem->head);
	free(mem);
}


// Hex or Dec representation of number to int
int string_to_int(char* string) {
	int result = 0;

	if (string[0] == '0' &&( string[1] == 'x' || string[1] == 'X'))
		result = strtoul(string + 2, NULL, 16); // Hex number given as an input  exmp .word 0x100 0x121423
	else
		result = atoi(string); // decnumber given

	return result;
}


// WordCommand - uses .word commands to save in memory ,pos get the address, pos1 & pos are final line index, pos will update pos1,
// returns updated memory line list
Memory_Line_list *WordCommand(Memory_Line_list* head, char line[MAX_LINE], int *last_pc) {
	char address[32], data[32];// TODO CHANGE SIZE
	int j = 0;   // index for insert string address and data
	int k = 0;   //  index for reading address data

	while (*line != ' ' && *line!= '\t' )// addvance to address(pass .word)
		line++;
	while (*line == ' '|| *line=='\t')//skip white spaces
		line++;

	while (line[k] != ' '&& line[k] != '\t') //Copy Address string
		address[j++] = line[(k)++];
	address[j] = '\0';

	while (line[k] == ' '|| line[k]== '\t')//skip white spaces till the data
		k++;

	j = 0;// now use for data
	while ((line[k] != ' ') && (line[k] != '\n')&& line[k] != '\t')// copy data string
		data[j++] = line[(k)++];
	data[j] = '\0';

	int address_pos = 0; // address_pos - address, can be hexadecimal or decimal
	address_pos = string_to_int(address);

	// save the command in the memory list. blank will be used as an indicator when writing to turn the command into a .word
	head = add_line_to_memory_list(head, "blank", "blank", "blank", "blank", data, address_pos);  //add line to line list.

	if (address_pos > *last_pc)
		*last_pc = address_pos;  // update the location of the end of the memory
	return head;
}


// reads opcode.  option - a string we copy the opcode to.
void read_opcode(char *line[MAX_LINE], char *opcode) {
	char *itr_line = *line;// itr line get the line from the pointer
	int j = 0; // index of copied char

	do { // reading opcode should continue till dollar of first register
		if (*itr_line != '$') {// if it's not dollar
			if ((*itr_line != '\t') && (*itr_line != ' ')) {// or whitespace
				opcode[j++] = *itr_line; // copy
			}
			*(itr_line++);
		}
	} while (*itr_line != '$');
	opcode[j] = '\0';
	*line = itr_line;// update line
}

// reads till dollar sign 
void readDollar(char* line[MAX_LINE]) {
	char *itr_line = *line;
	while (*itr_line != '$')
		itr_line++;
	*line = itr_line;// update line
}

// reads register name,  register_name it can be used for rd, rs or rt
void read_register_name(char* line[MAX_LINE], char* register_name) {
	char *itr_line = *line;// itr line get the line from the pointer
	int j = 0; // index register_name string

	while (*itr_line != ',')
	{
		if (*itr_line != ' ' && *itr_line != '\t') { // read if not a whitespace
			register_name[j] = *itr_line;
			j = j + 1;
		}
		itr_line++;
	}
	register_name[j] = '\0';
	*line = itr_line;// update line
}


// reads immediate value
void readImmd(char *line[MAX_LINE], char *imm) {
	char *itr_line = *line;// itr line get the line from the pointer

	while ((*itr_line == ' ') || (*itr_line == '\t') || (*itr_line == ',')) {// go to immediate
		itr_line++;
	}
	int j = 0; // index of char being copied in immediate string
	while (*itr_line != ' ')
	{
		if (*itr_line != ' ' && *itr_line != '\t') {
			if ((*itr_line == '\t') || (*itr_line == '\n'))
				break;
			imm[j++] = *itr_line;
		}
		itr_line++;
	}
	imm[j] = '\0';
	*line = itr_line;// update line
}

// reads line and adds to memory line list
// last_line - number of last line of memin , current_char_index - index of char being read. i-current instruction line index.
Memory_Line_list *readLine(char *line, int *last_line, int *current_position, Memory_Line_list *head) {
	char opcode[8 + 1], rd[6], rs[6], rt[6], imm[51];    // Iformat
	read_opcode(&line, opcode);		  // read the opcode
	read_register_name(&line, rd);    // Read rd register
	readDollar(&line);                // wait for dollar sign
	read_register_name(&line, rs);    // Read rs register
	readDollar(&line);                // wait for dollar sign
	read_register_name(&line, rt);    // Read rt register
	readImmd(&line, imm);             //handle immediate
	head = add_line_to_memory_list(head, opcode, rd, rs, rt, imm, *current_position);	// save line to line list
	(*current_position)++;
	if (*current_position > *last_line)
		*last_line = *current_position;  //Update last line position
	return head;
}


// the second iteration - reads each line on file and copies it to memory list
Memory* SecondRun(FILE* assembly) {
	int  current_position = 0, last_line = 0;// current position in the file(PC), last line- the last line of the file (hold last pc of a file)
	char* iter_line = NULL;
	char line[MAX_LINE] , temp_line[MAX_LINE];
	Memory_Line_list* head = NULL;  // the Memory list's head. it will contain info about each memory line in the end

	while ((fgets(line, MAX_LINE, assembly)) != NULL) { // the loop reads the file lines upon reaching null it stops as that's where the file ends
		clean_comment(line);
		strcpy(temp_line, line);
		clean_line(temp_line);
		if ( strcmp(temp_line, "\0") == 0 )// in case of a Blank line skip line // TODO
			continue;

		if ((iter_line=strstr(line, ".word")) != NULL) { // in case of the .word memory order
			head = WordCommand(head, iter_line, &last_line); // add to memory list
		}
		else {
			iter_line = strstr(line, ":");
			if (iter_line != NULL) {     //In the case is not ".word" check if it is only a label or also instruction
				iter_line++;
				clean_white_spaces(iter_line);

				if (current_position > last_line) //Update last line position
					last_line = current_position;
			}
			else {// it is only instruction line
				iter_line = line;
				clean_white_spaces(iter_line);
			}

			if (*iter_line == '\n' || *iter_line == '\0')
				continue;

			head = readLine(iter_line, &last_line, &current_position, head);// read instruction line and add to the memory list
		}
	}
	return create_mem(head, last_line);  // create memory structure and return it to main function
}
