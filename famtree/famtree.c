//Name: Autumn Henderson
//Date: September 7th, 2020	    Revised Date: November 10th, 2020
//Description/Notes: This program takes a text file on standard input and prints out the
//family tree of the family members in the file. It prints out a person's parents first,
//folled by that person. If there's a cycle in the tree, the program identifies the issue and
//exits. The program also allows for redundancy and infers where it can. For example, if after
//the person the identifier is "FATHER_OF", it can be assumed this person is "Male" and will
//assign this.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jrb.h"
#include "dllist.h"
#include "fields.h"
#include "jval.h"

typedef struct Person {
	char *Name;
	char Sex;
	char *Father;
	char *Mother;
	Dllist Children;
	int visited;
	int printed;
} Person;

//Function Make_Name takes an inputstruct and creates a name out of the fields
//in inputstruct. It returns a char*
char *Make_Name(IS is) {

	char *name;
	int i, size;

	//Determine size of name
	size = strlen(is->fields[1]);
	for (i = 2; i < is->NF; i++) size += (strlen(is->fields[i]) + 1);
	name = (char *)malloc(sizeof(char)*(size + 1));

	//Copy words into name
	strcpy(name, is->fields[1]);
	for (i = 2; i < is->NF; i++) {
		strcat(name, " ");
		strcat(name, is->fields[i]);
	}

	return name;
}

//Function Create_New_Person takes a struct Person and a char* name and initializes
//information for that person. It is a helper function to Insert_Person()
void Create_New_Person(Person *Family_Member, char *name) {


	Family_Member->Name = name;

	//Assigns rest of variables
	Family_Member->Sex = 'X';
	Family_Member->Father = "Unknown";
	Family_Member->Mother = "Unknown";
	Family_Member->Children = new_dllist();
	Family_Member->visited = 0;
	Family_Member->printed = 0;
}

//Function Insert_Person takes a JRB Family_Tree, a struct Person, and a char* and
//adds the Person to the Family_Tree
void Insert_Person(JRB Family_Tree, Person *Family_Member, char *name) {
	Family_Member = (Person*) malloc(sizeof(Person));

	Create_New_Person(Family_Member, name);

	//Inserts into tree
	jrb_insert_str(Family_Tree, Family_Member->Name, new_jval_v(Family_Member));
}

//Function Print_Person takes a struct Person and prints information about that Person.
//It is a helper function for Print_Tree()
void Print_Person(Person *Family_Member) {

	Dllist ptr;

	//Name
	printf("%s\n", Family_Member->Name);
	
	//Sex
	printf("  Sex: ");
	if(Family_Member->Sex == 'X') printf("Unknown\n");
	else if(Family_Member->Sex == 'M') printf("Male\n");
	else printf("Female\n");

	//Parents
	printf("  Father: %s\n", Family_Member->Father);
	printf("  Mother: %s\n", Family_Member->Mother);
	
	//Children
	if (dll_empty(Family_Member->Children) == 1) printf("  Children: None\n");
	else {
		printf("  Children:\n");
		dll_traverse(ptr, Family_Member->Children) {
			printf("    %s\n", (char *)ptr->val.v);
		}
	}
	printf("\n");
	Family_Member->printed = 1;
}

//Function Print_Tree takes a JRB Family_Tree and prints out the tree
void Print_Tree(JRB Family_Tree) {
	JRB node, father_node, mother_node;
	Person *p, *p2, *mother, *father;
	Dllist ptr, child, toprint;
	char *name;
	toprint = new_dllist();
	int found;


	//Adds parentless people to queue
	jrb_traverse(node, Family_Tree) {
		p = (Person *) node->val.v;	
		if(strcmp(p->Father, "Unknown") == 0 && strcmp(p->Mother, "Unknown") == 0) {
			dll_append(toprint, new_jval_v(p));
		}
	}

	while(dll_empty(toprint) != 1) {

		//Top of the list
		ptr = dll_first(toprint);
		p = (Person *)ptr->val.v;
		
		//--Grab the parents--//
		//Father
		found = 0;
		jrb_traverse(father_node, Family_Tree) {

			father = (Person *)father_node->val.v;
			if(strcmp(father->Name, p->Father) == 0) {
				found = 1;
				break;
			}
		}
		if (found == 0) father = NULL;
		//Mother
		found = 0;
		jrb_traverse(mother_node, Family_Tree) {

			mother = (Person *)mother_node->val.v;
			if(strcmp(mother->Name, p->Mother) == 0) {
				found = 1;
				break;
			}
		}
		if (found == 0) mother = NULL;

		//If the person has not been printed yet
		if (p->printed == 0) {
		
			//Checks for whether parents have been printed first
			if((father == NULL || mother == NULL) || ((father->printed == 1 && mother->printed == 1))) {

				//Print Person
				Print_Person(p);
				
				//Find Child Information and add to top of queue
				dll_traverse(child, p->Children) {
					name = child->val.v;

					jrb_traverse(node, Family_Tree) {
						p2 = (Person *)node->val.v;
						if (strcmp(p2->Name, name) == 0) {
							dll_append(toprint, new_jval_v(p2));
							break;
						}
					}
				}
			}
		}
		dll_delete_node(ptr);
	}
	free_dllist(toprint);
}

//Function is_decendant takes a JRB Family_Tree and a char* and 
//identifies cycles in the Family_Tree
int is_decendant(JRB Family_Tree, char *name) {

	JRB node;
	Person *person;
	char *child;
	Dllist ptr, children;

	//Gather child's information from tree
	node = jrb_find_str(Family_Tree, name);
	person = (Person *)node->val.v;

	children = person->Children;

	if(person->visited == 1) return 0;
	if(person->visited == 2) return 1;
	person->visited = 2;

	dll_traverse(ptr, children) {
		child = ptr->val.v;
		if (is_decendant(Family_Tree, child)) return 1;
	}

	person->visited = 1;
	return 0;
}


/*-----Main Function-----*/

int main() {

	IS is;
	char *name, *name2;
	char blank_line[2] = "\n";
	Person *Family_Member, *current;
	JRB Family_Tree, node;
	int  i, name_compare;
	Dllist ptr, ptr2;
	char *Childs_Name;
	int name_found;

	/* Allocate the IS and Family Tree*/

	is = new_inputstruct(NULL); //Null fo reading in stdin
	Family_Tree = make_jrb();

	//Reads in lines
	while(get_line(is) >= 0) {

		//Checks for blank line
		if (strcmp(is->text1, "\n") == 0) continue;
	
		
		
		/*---Testing first word for FATHER and MOTHER---*/
		
		//Father
		if(strcmp(is->fields[0],"FATHER") == 0) {	
	
			//Create name for link
			name2 = Make_Name(is);

			//ERROR-CHECK: Person has a different father assigned
			node = jrb_find_str(Family_Tree, name);
			current = (Person *) node->val.v;

			if (strcmp(current->Father, "Unknown") != 0 && strcmp(current->Father, name2) != 0) {
				fprintf(stderr, "Bad input -- child with two fathers on line %i\n", is->line);
				exit (1);
			}
				
			//Updates Child (Current Person)
			node = jrb_find_str(Family_Tree, name);
			current = (Person *) node->val.v;
			current->Father = name2;
	
			//Search tree for name and inserts if not there, then creates the link as 
			//the link should not exist if one of the people did not exist
			node = jrb_find_str(Family_Tree, name2);
			if (node == NULL) {
			
				//Insert and Update Father
				Insert_Person(Family_Tree, Family_Member, name2);
				node = jrb_find_str(Family_Tree, name2);
				current = (Person *) node->val.v;
				dll_append(current->Children, new_jval_s(strdup(name)));
				current->Sex = 'M';
			}
			//Otherwise, tests links to make sure they are valid and creates them if not
			else {
				
				//Invalid links: If person being identified as Father is female
				node = jrb_find_str(Family_Tree, name2);
				current = (Person *) node->val.v;

				if (current->Sex == 'F') {
					fprintf(stderr, "Bad input - sex mismatch on line %i\n", is->line);
					exit (1);
				}
				else {
					name_found = 0;
					dll_traverse(ptr, current->Children) {

						Childs_Name = (char *) ptr->val.v;
						if(strcmp(Childs_Name, name) == 0) {
							name_found = 1;
							break;
						}
					}
					if (name_found == 0) dll_append(current->Children, new_jval_s(strdup(name)));
					current->Sex = 'M';
				}
			}
		}

		//Mother
		if(strcmp(is->fields[0], "MOTHER") == 0) {	
			
			//Create name for link
			name2 = Make_Name(is);

			//Error check to make sure Current Person does not already have a mother
			node = jrb_find_str(Family_Tree, name);
			current = (Person *) node->val.v;

			if (strcmp(current->Mother, "Unknown") != 0 && strcmp(current->Mother, name2) != 0) {
				fprintf(stderr, "Bad input -- child with two mothers on line %i\n", is->line);
				exit(1);
			}
			
			//Updates Child (Current Person)
			node = jrb_find_str(Family_Tree, name);
			current = (Person *) node->val.v;
			current->Mother = name2;

			//Search tree for name and inserts if not there, then creates the link as 
			//the link should not exist if one of the people did not exist
			node = jrb_find_str(Family_Tree, name2);
			if (node == NULL) {
				
				//Insert and Update Mother
				Insert_Person(Family_Tree, Family_Member, name2);
				node = jrb_find_str(Family_Tree, name2);
				current = (Person *) node->val.v;
				dll_append(current->Children, new_jval_s(strdup(name)));
				current->Sex = 'F';
			}
			//Otherwise, tests links to make sure they are valid and creates them if not
			else {
				
				//Invalid links: If person being identified as Mother is male
				node = jrb_find_str(Family_Tree, name2);
				current = (Person *) node->val.v;

				if (current->Sex == 'M') {
					fprintf(stderr, "Bad input - sex mismatch on line %i\n", is->line);
					exit(1);
				}
				else {
						
					name_found = 0;
					dll_traverse(ptr, current->Children) {

						Childs_Name = (char *) ptr->val.v;
						if(strcmp(Childs_Name, name) == 0) {
							name_found = 1;
							break;
						}
					}
					if (name_found == 0) dll_append(current->Children, new_jval_s(strdup(name)));
					current->Sex = 'F';
				}
			}
		}
		


		/*---Testing first word for FATHER_OF and MOTHER_OF---*/

		//FATHER_OF
		if (strcmp(is->fields[0], "FATHER_OF") == 0) {

			//ERROR-CHECK: Father's Sex
			node = jrb_find_str(Family_Tree, name);
			current = (Person *)node->val.v;

			if (current->Sex == 'F') {
				fprintf(stderr, "Bad input - sex mismatch on line %i\n", is->line);
				exit (1);
			}

			current->Sex = 'M';		//Automatically assumes 'Male' for Father_Of

			//Create second name for child
			name2 = Make_Name(is);

			//Search tree for child and inserts if the child was not there.
			node = jrb_find_str(Family_Tree, name2);
			//If the child does not exist, creates the child and inserts into the tree
			if (node == NULL) {
				
				//Insert and update child
				Insert_Person(Family_Tree, Family_Member, name2);
				node = jrb_find_str(Family_Tree, name2);
				current = (Person *) node->val.v;
				current->Father = name;

				//Insert Child into Father's list
				node = jrb_find_str(Family_Tree, name);
				current = (Person *) node->val.v;
				dll_append(current->Children, new_jval_s(strdup(name2)));
			}
			//Otherwise, test links to make sure thay are valid and create them if not
			else {
				
				node = jrb_find_str(Family_Tree, name2);
				current = (Person *) node->val.v;

				//If the value of the child's father is not Unknown and it's not the name, then it's a different name
				if (strcmp(current->Father, "Unknown") != 0 && strcmp(current->Father, name) != 0) {
					fprintf(stderr, "Bad input -- child with two fathers on line %i\n", is->line);
					exit(1);
				}
				current->Father = name;

				//Checks for child in Father's child list. If not there, inserts the child.
				node = jrb_find_str(Family_Tree, name);
				current = (Person *) node->val.v;

				name_found = 0;
				dll_traverse(ptr, current->Children) {

					Childs_Name = (char *) ptr->val.v;
					if(strcmp(Childs_Name, name2) == 0) {
						name_found = 1;
						break;
					}
				}
				if (name_found == 0) dll_append(current->Children, new_jval_s(strdup(name2)));
			}
		}
		
		//MOTHER_OF
		if (strcmp(is->fields[0], "MOTHER_OF") == 0) {

			//ERROR-CHECK: Mother's Sex
			node = jrb_find_str(Family_Tree, name);
			current = (Person *)node->val.v;

			if (current->Sex == 'M') {
				fprintf(stderr, "Bad input - sex mismatch on line %i\n", is->line);
				exit (1);
			}

			current->Sex = 'F';		//Automatically assumes 'Female'

			//Create second name for child
			name2 = Make_Name(is);

			//Search tree for child and inserts if the child was not there.
			node = jrb_find_str(Family_Tree, name2);
			//If the child does not exist, creates the child and inserts into the tree
			if (node == NULL) {
		
				//Insert and update child
				Insert_Person(Family_Tree, Family_Member, name2);
				node = jrb_find_str(Family_Tree, name2);
				current = (Person *) node->val.v;
				current->Mother = name;

				//Insert Child into Father's list
				node = jrb_find_str(Family_Tree, name);
				current = (Person *) node->val.v;
				dll_append(current->Children, new_jval_s(strdup(name2)));
			}
			//Otherwise, test links to make sure thay are valid and create them if not
			else {
		
				node = jrb_find_str(Family_Tree, name2);
				current = (Person *) node->val.v;

				//If the value of the child's mother is not Unknown and it's not the name, then it's a different name
				if (strcmp(current->Mother, "Unknown") != 0 && strcmp(current->Mother, name) != 0) {
					fprintf(stderr, "Bad input -- child with two mothers on line %i\n", is->line);
					exit(1);
				}
				current->Mother = name;

				//Checks for child in Mother's child list. If not there, inserts the child.
				node = jrb_find_str(Family_Tree, name);
				current = (Person *) node->val.v;

				name_found = 0;
				dll_traverse(ptr, current->Children) {

					Childs_Name = (char *) ptr->val.v;
					if(strcmp(Childs_Name, name2) == 0) {
						name_found = 1;
						break;
					}
				}
				if (name_found == 0) dll_append(current->Children, new_jval_s(strdup(name2)));
			}	
		}

		

		/*---Testing first word for SEX---*/

		if (strcmp(is->fields[0], "SEX") == 0) {
			
			node = jrb_find_str(Family_Tree, name);
			current = (Person *) node->val.v;

			//SEX = M
			if (strcmp(is->fields[1], "M") == 0) {

				//Checks current assignage
				if(current->Sex == 'F') {
					fprintf(stderr, "Bad input - sex mismatch on line %i\n", is->line);
					exit(1);
				}
				current->Sex = 'M';
			}
			//Sex = F
			if (strcmp(is->fields[1], "F") == 0) {
				
				//Checks current assignage
				if(current->Sex == 'M') {
					fprintf(stderr, "Bad input - sex mismatch on line %i\n", is->line);
					exit(1);
				}
				current->Sex = 'F';
			}
		}



		/*---Testing first word for PERSON---*/
	
		if (strcmp(is->fields[0], "PERSON") == 0) {
		
			//Assigns name
			name = Make_Name(is);

			//Test to see if person is in tree and inserts them if not
			node = jrb_find_str(Family_Tree, name);
			if (node == NULL) Insert_Person(Family_Tree, Family_Member, name);
		}
	}



	/*---Test for Cycles---*/
	
	jrb_traverse(node, Family_Tree) {
		
		current = (Person *) node->val.v;
		name = current->Name;

		if(is_decendant(Family_Tree, name) == 1) {
			fprintf(stderr, "Bad input -- cycle in specification\n");
			exit(1);
		}
	}



	/*---Print Tree---*/
	
	Print_Tree(Family_Tree);



	/*---Free Memory---*/

	jettison_inputstruct(is);
	jrb_free_tree(Family_Tree);
	return 0;
}
