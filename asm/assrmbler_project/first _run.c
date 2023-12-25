#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "first _run.h"





// this function creates and initilize label with  the given data , location is a PC
label* create_label(char* name, int pc)
{
	label* new_label = (label*)malloc(sizeof(label));
	if (new_label != NULL) { // if allocation successful. insert data
		strcpy(new_label->name, name);
		new_label->address = pc;
		new_label->next = NULL;
	}
	return new_label;
}


// will be used in the --first-- iteration
label* add_label_to_list(label* head, char* name, int PC) {

	label* new_label = create_label(name, PC);// create new label and check if successed. otherwise return a null
	if (new_label == NULL)
		return NULL;

	new_label->next = head;
	return new_label;
}


// free the list and frees the memory
void free_label_list(label* head)
{
	label* temp;
	while (head != NULL)
	{
		temp = head;
		head = head->next;
		free(temp);
	}
}


//get the the pointer to the Character after ":"
int also_instruction(char* line) {
	if (strstr(line, "$"))
		return 1;
	return 0;
}


// the code of the first iteration, make the label list.
label* first_run(FILE *assembly) {
	int rowIndex = 0;  //the code row's index. where the PC will go after reading the label
	int counter = 0;  // counter will be the line number (pc)
	int char_index, label_string_index;
	int option;     // option is a code which determining if it's a label only line or a label + command line

	char tav;            //  tav - current char when reading label name
	char line[MAX_LINE];  // line the current line being read
	char lable_name[MAX_LABEL_LEN];	// label_line will contain the label name

	label* head = NULL;  // the label list's head
	while (!feof(assembly)) {
		fgets(line, MAX_LINE, assembly);   // read a command from the assembler file

		clean_line(line); //clean comment and White characters
		option = 0;  // reset option

		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) //If line is blank
			continue;
		if (strstr(line, ".word") != NULL)//If line is .word, continue
			continue;

		if (strstr(line, ":") != NULL) {//If dots are found, this is a label
			char_index = 0; label_string_index = 0;  //Read the label name, first reset indexes
			do {
				tav = line[char_index];  // get current char from the line
				if (tav != ':') {		   // two dots is where it ends so skip
					lable_name[label_string_index++] = tav;  // grab the read char and put it in name string
					char_index++;
				}
			} while (tav != ':');
			lable_name[label_string_index] = '\0';

			// Check if the line is lable line only or also instuction line
			char_index++; //hold the index after the  ":"
			option = also_instruction(&line[char_index]);         // option is 0 on only a label , otherwise is also instruction 1
			head = add_label_to_list(head, lable_name, counter);  // add the label to label list

			if (option == 0)   // Only label line - add label and decrement counter
				counter = counter - 1;
		}
		counter++;  // increment file line counter
	}
	return head; // return the list
}




// this will be used in the --second-- iteration
// this will tun over the label list and look for "name". it will return it's location.
int find_label(label* label_list, char* label_to_find) {

	if (label_list==NULL)
	{
		return -1;
	}
	while (strcmp(label_list->name, label_to_find) != 0) {// ass long they are unequal
		label_list = label_list->next;//move forward

		if (label_list == NULL) { // if we get to the end of the list its mean we not found so return -1
			return -1;
		}
	}
	// return the current's location
	return label_list->address;
}


// function that runs after the second run and changes label names in the memory list to thier location PC.
// if $zero was recorded in the immediate field the function also changes immediate value to 0 
void LableChange(Memory_Line_list* memory_list, label* label_list)
{
	char temp[MAX_LABEL_LEN];

	Memory_Line_list *iter_memory_list = memory_list;// iterator on memory line list
	while (iter_memory_list != NULL) {

		int location = find_label(label_list, iter_memory_list->imm);// find label in the list and return label's location if dosnt exist return -1

		if (location != -1) {// if found
			_itoa(location, temp, 10); // convert int to a string and put it in temp
			strcpy(iter_memory_list->imm, temp);
		}

		if (strcmp(iter_memory_list->imm, "$zero") == 0) { // handle  immediate $zero case
			strcpy(iter_memory_list->imm, "0"); // Changes immidiate to "0"
		}
		iter_memory_list = iter_memory_list->next;
	}
}


void clean_line(char* line) // clear all white chars and none informative data
{
	clean_comment(line);
	clean_white_spaces(line);
}
void clean_comment(char* line)
{
	char* hashtag = strstr(line, "#");
	if (hashtag != NULL)// if there is a cooment > get rid
		*hashtag = '\0';
}
void clean_white_spaces(char* line)
{
	char new_line[MAX_LINE];
	int i = 0, j = 0;
	while (line[i] != '\0')
	{
		if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
			i++;
			continue;
		}
		else {
			new_line[j++] = line[i++];

		}
	}
	new_line[j] = '\0';
	strcpy(line, new_line);
}
