//Autumn Henderson
//November 6th, 2020
//mymalloc.c
//Program implements malloc, free, and coalesce

#include "mymalloc.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct flist {
	int size;
	struct flist *prev;
	struct flist *next;
} *Flist;

Flist malloc_head = NULL;


void *my_malloc(size_t size) {

	void *ptr, *free_ptr, *user_ptr;
	Flist free_chunk, tmp_chunk, tmp_chunk2, tmp_chunk3, previous, next;
	int remainder_size;

	//Adjust Size
	if (size % 8 != 0) size = size + (8 - (size % 8));	//Pads to multiple of 8
	size += 8;		//bookkeeping

	//No Chunks on free list to give
	if (malloc_head == NULL) {

		//If the user asks for less than 8192, sbrk(8192) and put remainder on free list
		if (size < 8192) {
			ptr = sbrk(8192);
			free_chunk = (Flist)ptr;
			*(int *)ptr = size;

			//Add free_chunk to the list
			free_chunk->prev = NULL;
			free_chunk->next = NULL;
			free_chunk = ptr + size;
			free_chunk->size = 8192 - size;
			malloc_head = free_chunk;
		}
		
		//Else size is greater than 8192, thus give the user everything
		else {
			ptr = sbrk(size);
			*(int *)ptr = size;
		}

		return ptr += 8;
	}

	//Examine possible chunks on free list
	else {
		tmp_chunk = malloc_head;	//Assign chunk to the top of the free list
		free_ptr = tmp_chunk;		//Assign the free ptr to the chunk
		
		while (1) {
			//Condition 1: Next available chunk is enough for the user
			if (tmp_chunk->size >= size) {
				
				//Determine size to be put back into tmp_chunk
				remainder_size = tmp_chunk->size - size;

				//Next available chunk is exactly enough as there's no remainder to put back in
				if (remainder_size == 0) {
					//Fix the list to remove the whole chunk

					//If there was only one entry
					if (tmp_chunk->prev == NULL && tmp_chunk->next == NULL) malloc_head = NULL;
					//Else there were more entries, but it was the top entry
					else if (tmp_chunk->prev == NULL && tmp_chunk->next != NULL) malloc_head = tmp_chunk->next;
					//Else there where more entries, but it was the bottom entry
					else if (tmp_chunk->next == NULL && tmp_chunk->prev != NULL) tmp_chunk->prev->next = NULL;
					//Else there were more entries and it was inbetween two of them
					else if (tmp_chunk->next != NULL && tmp_chunk->prev != NULL) {
						tmp_chunk2 = tmp_chunk->prev;
						tmp_chunk3 = tmp_chunk->next;
						tmp_chunk2->next = tmp_chunk3;
						tmp_chunk3->prev = tmp_chunk2;
					}

					//Return the memory
					*(int *)free_ptr = size;
					return free_ptr += 8;
				}
				//Next available chunk is more than enough
				else {
					free_ptr = tmp_chunk;
					user_ptr = tmp_chunk;
					
					//Work on adding new chunk to free list
					previous = tmp_chunk->prev;
					next = tmp_chunk->next;
				
					//Adjust the free chunk's information
					free_ptr += size;
					free_chunk = (Flist)free_ptr;
					free_chunk->size = remainder_size;
					free_chunk->prev = tmp_chunk->prev;
					free_chunk->next = tmp_chunk->next;

					//If there were previous and/or next chunks, hook in free chunk
					if (previous != NULL) previous->next = free_chunk;
					if (next != NULL) next->prev = free_chunk;

					//If the new free_chunk should be at the top, readjust malloc_head
					if (free_chunk->prev == NULL) malloc_head = free_chunk;

					//Ptr being returned to the user
					*(int *)user_ptr = size;
					return user_ptr += 8;
				}
			}
			
			//Condition 2: There are no chunks of the right size, so sbrk is used
			else if(tmp_chunk->next == NULL) {
				if (size < 8192) {
					user_ptr = sbrk(8192);
					free_chunk = (Flist)user_ptr;
					*(int *)user_ptr = size;

					//Add free_chunk to the list
					free_chunk->next = NULL;
					free_chunk = user_ptr + size;
					free_chunk->size = 8192 - size;
					
					tmp_chunk->next = free_chunk;
					free_chunk->prev = tmp_chunk;
				}
				else {
					user_ptr = sbrk(size);
					*(int *)user_ptr = size;
				}

				return user_ptr += 8;
			}

			//Condition 3: Move on to the next chunk
			else tmp_chunk = tmp_chunk->next;
		}
	}
}

void my_free(void *ptr) {

	Flist free_chunk, tmp_chunk;

	//Move back 8 for bookkeeping information
	ptr -= 8;

	//Set up free chunk
	free_chunk = (Flist) ptr;
	free_chunk->size = *(int *)ptr;
	free_chunk->prev = NULL;	//Chunk is returned to the top of the list, so previous should be NULL
	free_chunk->next = NULL;	//Giving value NULL

	//Malloc_Head could equal NULL under the circumstance that the full chunk of sbrk was returned
	if (malloc_head == NULL) malloc_head = free_chunk;
	//Else the free chunk needs to be added back
	else {
		tmp_chunk = malloc_head;
		malloc_head = free_chunk;
		tmp_chunk->prev = free_chunk;
		free_chunk->next = tmp_chunk;
	}
}

void *free_list_begin() { return malloc_head; }

void *free_list_next(void *node) {
	Flist tmp;
	tmp = (Flist)node;
	return tmp->next;
}

int comparator (const void *p1, const void *p2) { 
	return (*(int*)p1 - *(int*)p2); 
}

void coalesce_free_list() {

	Flist chunk, tmp_chunk, next;
	Flist *array;
	int num_nodes, i, size;
	void *ptr;
	int *node;

	//Nothing on the list to coalesce
	if (malloc_head == NULL || malloc_head->next == NULL) return;

	//Count up number of chunks to merge
	chunk = malloc_head;
	num_nodes = 0;
	while (chunk != NULL) {
		chunk = chunk->next;
		num_nodes++;
	}

	//Create array and store nodes
	array = (Flist *) malloc(sizeof(Flist *)*num_nodes);
	chunk = malloc_head;

	i = 0;
	while(i < num_nodes) {
		array[i] = chunk;
		chunk = chunk->next;
		i++;
	}
	
	qsort(array, num_nodes, sizeof(Flist*), comparator);

	//Link everything together
	i = 1;
	while(i < num_nodes - 1) {
		array[i]->next = array[i+1];
		array[i]->prev = array[i-1];
		i++;
	}
	//Adust first and last nodes
	array[0]->next = array[1];
	array[0]->prev = NULL;
	array[num_nodes-1]->prev = array[num_nodes-2];
	array[num_nodes-1]->next = NULL;
	malloc_head = array[0];


	free(array);
	
	//Coalesce
	chunk = malloc_head;
	ptr = malloc_head;

	while(chunk != NULL) {
		size = *(int*)ptr;
		node = (int*) (ptr + size);
		tmp_chunk = (Flist) node;
		
		if(tmp_chunk == chunk->next) {
			*(int*)ptr = size + *(int*)node;
			next = tmp_chunk->next;
			chunk->next = next;
		}
		else {
			chunk = chunk->next;
			ptr = (void*)chunk;
		}
	}
	
	//Link all prev's
	chunk = malloc_head->next;	
	tmp_chunk = malloc_head;

	while(chunk != NULL) {
		chunk->prev = tmp_chunk;
		tmp_chunk = chunk;
		chunk = chunk->next;
	}
}
