#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "first _run.h"
#include "second_run.h"
#include "output.h"




// THE main function is executing the assembeler  
int main(int argc, char* argv[]) {

	FILE *assembly = fopen(argv[1], "r");
	if (assembly == NULL) {
		exit(1);
	}

	label* labels = first_run(assembly);// the first iteration, locate the labels and write thier locations to the linked list
	rewind(assembly);// return file pointer to the head of a file

	Memory *memory = SecondRun(assembly);// start the second iteration
	fclose(assembly);

	LableChange(memory->head, labels); // Change labels from words to numbers

	FILE* memin = fopen(argv[2], "w");// Write Data to file
	if (memin == NULL)
		exit(1);
	print_data_file(memory, memin);
	fclose(memin);// End of file writing

	// free the memory taken by the label list and memory structure
	free_label_list(labels);
	free_memory_struct(memory);
}



