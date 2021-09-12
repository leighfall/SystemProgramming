//Name: Autumn Henderson
//Date: September 29th, 2020
//Description: Like make, fakemake automates compiling. It limits itself to making
//one executable, and it assumes that you are using gcc to do compilation
//
//FIXME Please grade this with the late penalty and not the late waiver.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "fields.h"
#include "dllist.h"

//Function Print_List takes a Dllist and prints its contents. Used for debugging.
void Print_List(Dllist list) {

	Dllist ptr;
	char* filename;

	printf("Printing file list: ");
	dll_traverse(ptr, list) {
		filename = (char *) ptr->val.v;
		printf("%s ", filename);
	}

	printf("\n\n");
}

//Function Print_Oline takes a Dllist and a char* and prints the gcc -c for
//.o files
char* Print_Oline(Dllist ffiles, char* cfilename) {

	char* gccoline;
	Dllist ptr;

	gccoline = malloc(1000);

	strcpy(gccoline, "gcc -c ");

	dll_traverse(ptr, ffiles) {

		strcat(gccoline, (char*)ptr->val.v);
		strcat(gccoline, " ");
	}

	strcat(gccoline, cfilename);

	printf("%s\n", gccoline);

	return gccoline;
}

//Function Print_Executable takes 3 Dllists and a char* and composes the executable
//line.
char* Print_Executable(Dllist ffiles, Dllist ofiles, Dllist lfiles, char *executable) {

	char* full_executable;
	Dllist ptr;
	full_executable = malloc(1000);
	
	strcpy(full_executable, "gcc -o ");
	strcat(full_executable, executable);
	dll_traverse(ptr, ffiles) {
		strcat(full_executable, " ");
		strcat(full_executable, (char *)ptr->val.v);
	}

	dll_traverse(ptr, ofiles) {
		strcat(full_executable, " ");
		strcat(full_executable, (char *)ptr->val.v);
	}

	dll_traverse(ptr, lfiles) {
		strcat(full_executable, " ");
		strcat(full_executable, (char *)ptr->val.v);
	}

	printf("%s\n", full_executable);

	return full_executable;
}

int main(int argc, char **argv) {

	char *filename, *executable, *line, *ofilename, *gccc, *gcco, *full_executable;
	char blank_line[2] = "\n";	//For comparing blank lines
	int i;
	IS is;						//Input struct
	Dllist cfiles;
	Dllist hfiles;
	Dllist lfiles;
	Dllist ffiles;
	Dllist ofiles;
	Dllist ptr;					//For traversing dllist
	int e_exist;				//flag
	int maxh, maxo, cfilestime;	//Records max times
	struct stat h;
	struct stat executable_time;

	
	
	/* Open File */
	
	if (argc < 2) {
		
		is = new_inputstruct("fmakefile");
		if (is == NULL) {
			fprintf(stderr, "fakemake: fmakefile No such file or directory\n");
			exit (1);
		}
	}
	else {
		filename = argv[1];
		is = new_inputstruct(filename);
	}

	if (is == NULL) {
		fprintf(stderr, "fakemake: No such file or directory\n");
		exit (1);
	}



	/* Process Lines in File */
	
	e_exist = 0;
	cfiles = new_dllist();
	hfiles = new_dllist();
	lfiles = new_dllist();
	ffiles = new_dllist();

	while (get_line(is) >= 0) {
		
		//If it is a blank line, do nothing
		if (strcmp(is->text1, blank_line) == 0) continue;
		
		//If line starts with "C", add to the .c files list
		if (strcmp("C", is->fields[0]) == 0) {
			for (i = 1; i < is->NF; i++) dll_append(cfiles, new_jval_s(strdup(is->fields[i])));
		}

		//If line starts with "H", add to the Header files list
		else if (strcmp("H", is->fields[0]) == 0) {
			for (i = 1; i < is->NF; i++) dll_append(hfiles, new_jval_s(strdup(is->fields[i])));
		}

		//If line starts with "L", add to the Library list
		else if (strcmp("L", is->fields[0]) == 0) {
			for (i = 1; i < is->NF; i++) dll_append(lfiles, new_jval_s(strdup(is->fields[i])));
		}

		//If line starts with "F", add to the Flags list
		else if (strcmp("F", is->fields[0]) == 0) {
			for (i = 1; i < is->NF; i++) dll_append(ffiles, new_jval_s(strdup(is->fields[i])));
		}

		//If line starts with "E", Create Executable name
		else if (strcmp("E", is->fields[0]) == 0) {
			e_exist++;
		
			//ERROR-CHECK: There is more than one E line
			if(e_exist > 1) {
				fprintf(stderr, "fmakefile (%i) cannot have more than one E line\n", is->line);
				exit (1);
			}

			executable = malloc(sizeof(char)* strlen(is->fields[1]));
			strcpy(executable, is->fields[1]);
		}
		
		//ERROR-CHECK: Else it is an unprocessed line
		else {
			fprintf(stderr, "Unprocessed line\n");
			exit (1);
		}
	}

	//ERROR-CHECK: No E line (ie. no executable)
	if(e_exist == 0) {
		fprintf(stderr, "No executable specified\n");
		exit (1);
	}



	/* Process Header Files */
	
	maxh = 0;
	dll_traverse(ptr, hfiles) {
		
		//ERROR-CHECK: Header file does not exist 
		if (stat((char *)ptr->val.v, &h) == -1) {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", (char *)ptr->val.v);
			exit (1);
		}
		//Else, compare current header file's time and if it is greater than maxh, replace
		else if (h.st_mtime >= maxh) maxh = h.st_mtime;
	}



	/* Process C Files */

	cfilestime = 0;
	ofiles = new_dllist();
	dll_traverse(ptr, cfiles) {

		//ERROR-CHECK: .c file does not exist
		if (stat((char *)ptr->val.v, &h) == -1) {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", (char *) ptr->val.v);
			exit (1);
		}

		//Composes the ofilename
		ofilename = malloc(sizeof(char) *strlen((char *)ptr->val.v));
		strcpy(ofilename, (char *)ptr->val.v);
		ofilename = strtok(ofilename, "c");
		strcat(ofilename, "o");
		dll_append(ofiles, new_jval_s(ofilename));

		cfilestime = h.st_mtime;
		//Calls stat on the ofilename to "look" for the file
		if(stat(ofilename, &h) == -1) {
			gccc = Print_Oline(ffiles, (char*)ptr->val.v);
		
			//ERROR-CHECK: system call fails
			if(system(gccc) != 0) {
				fprintf(stderr, "Command failed.  Exiting\n");
				exit (1);
			}

		}
		else {
			if(h.st_mtime < cfilestime || h.st_mtime < maxh) {
				gccc = Print_Oline(ffiles, (char*)ptr->val.v);
				
				//ERROR-CHECK: system call fails
				if(system(gccc) != 0) {
					fprintf(stderr, "Command failed.  Exiting\n");
					exit (1);
				}
			}
		}
	}

	

	/* Process the Executable */

	//Determines most recent .o file
	maxo = 0;
	dll_traverse(ptr, ofiles) {
		if(stat((char *) ptr->val.v, &h) == 0) if (h.st_mtime >= maxo) maxo = h.st_mtime;		
	}

	full_executable = malloc (1000);
	
	//If the executable exists
	if(stat(executable, &executable_time) == 0) {

		//If the executable is not as recent as the .o file, compile
		if (executable_time.st_mtime < maxo) {
			full_executable = Print_Executable(ffiles, ofiles, lfiles, executable);
			
			//ERROR-CHECK: Failure of executable
			if(system(full_executable) != 0) {
				fprintf(stderr, "Command failed.  Fakemake exiting\n");
				exit (1);
			}
			
		}
		//Else everything is up to date
		else {
			printf("%s up to date\n", executable);
			exit (1);
		}
	}
	//Else the executable does not exist, so compose and run system
	else {
		full_executable = Print_Executable(ffiles, ofiles, lfiles, executable);
		
		//ERROR-CHECK: Failure of executable
		if (system(full_executable) != 0) {
			fprintf(stderr, "Command failed.  Fakemake exiting\n");
			exit (1);
		}
	}



	/* Free Memory */

	free_dllist(cfiles);
	free_dllist(hfiles);
	free_dllist(lfiles);
	free_dllist(ffiles);
	free_dllist(ofiles);
	jettison_inputstruct(is);
	free(ofilename);
	free(full_executable);
	free(executable);
	return 0;
}
