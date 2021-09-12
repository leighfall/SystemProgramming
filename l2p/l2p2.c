//Name: Autumn Henderson
//Date: September 14th, 2020
//Description: This program opens the 'converted' file using system calls, processes the file into 
//a jrb tree, and allows for the user to search the tree for information on a machine given the local
//name of the machine.
//
//l2p2 and l2p3 are nearly identical as l2p1 was written with a buffer to begin with and l2p2 translated
//l2p1 except using system calls.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "jrb.h"
#include "dllist.h"
#include "jval.h"

//Machine struct
typedef struct Machine{
	unsigned char address[4];
	Dllist names;
} Machine;

//Function Print_Machine takes a Machine and returns a void. It prints
//information about the Machine struct
void Print_Machine(Machine *machine) {
	
	int i;
	i = 0;
	Dllist ptr;
	char *name;

	while(i < 3) {
		printf("%u.", machine->address[i]);
		i++;
	}
	printf("%u: ", machine->address[i]);

	dll_traverse(ptr, machine->names) {
		name = (char *) ptr->val.v;
		printf("%s ", name);
	}
	printf("\n\n");
}

//Function Num_Names takes an unsigned char array and returns an integer.
//It creates an integer that represents the individual characters in the array.
int Num_Names(unsigned char int_name[]) {

	int tens;
	int hunds;
	int thous;

	tens = int_name[2] * 10;
	hunds = int_name[1] * 100;
	thous = int_name[0] * 1000;

	return int_name[3] + tens + hunds + thous;
}


int main() {

	unsigned char buffer[350000];
	unsigned char int_names[4];
	char input[1000];
	char file_name[11] = "converted";
	char *names_of_machine, *local_name, *host_name;
	size_t size;
	int fp;
//	FILE* fp;
	int num_names;
	int i, j, k, l;
	Machine *machine, *current;
	int local_exist, host_name_exists;
	int word_compare;
	JRB All_Machines, node;
	Dllist dnode;	

	size = 0;
	num_names = 0;
	i = 0;
	All_Machines = make_jrb();


	//Reads in the file into a buffer
	
	//fp = fopen(file_name, "rb");
	fp = open(file_name, O_RDONLY);
	if (fp == -1) {
		perror ("l2p1: No such file or directory\n\n");
		return -1;
	}


	//size = fread(&buffer, sizeof(unsigned char), 350000, fp);
	l = 1;
	while (l > 0) {
		l = read(fp, buffer, 350000);
	}
	close(fp);

	//Processes file's contents
	//Size determined by l2p1
	while (i < 335063) {
		
		//Malloc for 'Machine'
		machine = (Machine*) malloc(sizeof(Machine));
		machine->names = new_dllist();
		
		//Read first four bytes and generate machine's address
		j = 0;
		while (j < 4) {
			machine->address[j] = buffer[i+j];
			j++;
		}

		i += 4;
		
		//Read next four bytes and generate the number of names to be read
		j = 0;
		while (j < 4) {
			int_names[j] = buffer[i+j];
			j++;
		}

		num_names = Num_Names(int_names);

		i += 4;


		//Read num_names names and adds machines into the tree
		
		j = 0;
		
		while (j < num_names) {
		
			names_of_machine = malloc(1000);

			k = 0;
			local_exist = 0;
			
			//Constructs the name until a null character is reached
			while(buffer[i] != '\0') {
				

				//Constructs a local name as well from an absolute name
				if(local_exist == 0 && buffer[i] == '.') {
					local_name = malloc(1000);
					strcpy(local_name, names_of_machine);
					local_exist = 1;
					dll_append(machine->names, new_jval_s(strdup(local_name)));
				}

				//Adds characters to name of machine
				names_of_machine[k] = buffer[i];
				k++;
				i++;
			}
			
			//Adds name (either local or absolute)
			dll_append(machine->names, new_jval_s(strdup(names_of_machine)));
			

			j++;
			i++;
		}

		//Machine is complete, needs to be inserted into tree
		
		dll_traverse (dnode, machine->names) {
			local_name = (char *) dnode->val.v;
			jrb_insert_str(All_Machines, local_name, new_jval_v(machine));
		}
	}

	printf("Hosts all read in\n\n");
	


	/* Retrieve Host Information */

	printf("Enter host name: ");

	while(1) {

		//Takes host name information from user

		if (scanf("%s", input) == EOF) break;
		host_name = strdup(input);

		host_name_exists = 0;
		jrb_traverse(node, All_Machines) {

			word_compare = strcmp(node->key.s, host_name);
			if (word_compare == 0) {
				current = (Machine *) node->val.v;
				Print_Machine(current);
				host_name_exists = 1;
			}
		}

		//Error: No key
		if (host_name_exists == 0) printf("no key %s\n\n", host_name);

		free(host_name);
		printf("Enter host name: ");
	}



	/* Frees Memory */
	jrb_free_tree(All_Machines);
	return 0;
}
