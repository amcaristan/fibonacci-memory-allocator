/********************************************************************************************************
 File: my_malloc.h
 
 Authors: Professor R. Bettati & Adrien Mombo-Caristan
 Department of Computer Science
 Texas A&M University
 Date: 01/31/2014
 
 This file contains the declarations for the my_allocator module as well as List 
 and Header structures for doubly-linked list implementation.
*********************************************************************************************************/


/*------------------------------------------------------------------------------------------------------*/
/* DEFINES */
/*------------------------------------------------------------------------------------------------------*/

#ifndef __Memory_Allocator__C___my_malloc__
#define __Memory_Allocator__C___my_malloc__
#define HEADER_IDENT 1138

/*------------------------------------------------------------------------------------------------------*/
/* INCLUDES */
/*------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

/*------------------------------------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*------------------------------------------------------------------------------------------------------*/

/* Header for storing memory block information */
typedef struct Header {
    unsigned short int header_ident; /* For identifying a Header element, a const */
    unsigned int block_count; /* Number of blocks this Header is responsible for, 
                                 multiplied w/ basic_block_size to find size of 
                                 block in bytes. Should be a fibonacci number */
    char is_free; /* 'Y'es or 'N'o */
    char child; /* 'L'eft or 'R'ight */
    char inherit; /* 'inherit' holds left child's parent's 'child' bit, and right 
                      child's parent's 'inherit' bit */
    struct Header *prev; /* pointer to previous memory block */
    struct Header *next; /* pointer to next memory block */
} Header;

/* Header for storing memory (buddy) block information */
typedef struct Efficient_Header {
    /* 'header' is a bit-mask that will be >= 32 bits and will hold the following (info:#bits):
        (header_ident:16 | free:1 | child:1 | inherit:1 | block_count:13+) */
    
    /* 'header_ident' validates a 'header' (const, set to a random number in call to init_allocator()) */
    /* 'block_count' holds #blocks this 'header' is responsible for. Should be a fibonacci number. 
        Multiply by basic_block_size to find the size of this header's block in bytes. */
    /* 'free' holds information on the availability of the block for allocation: yes == 1 , no == 0 */
    /* 'child' holds information on whether this header points to a left child (1) or right child (0) */
    /* 'inherit' holds left child's parent's 'child' bit, or the right child's parent's 'inherit' bit */
    uint_fast32_t header;
    struct Efficient_Header *prev; /* pointer to previous memory block in free_store */
    struct Efficient_Header *next; /* pointer to next memory block in free_store */
} Efficient_Header;

/*------------------------------------------------------------------------------------------------------*/
/* MODULE MY_MALLOC */
/*------------------------------------------------------------------------------------------------------*/


typedef void* Addr;


/* Return Fibonacci number closest to _min_number, if return_fib_index == 1, then
   return index of fibonnaci number in Fibonacci sequence, else if 
   return_fib_index == 0 return actual fibonnaci number */
unsigned int find_fibonacci(unsigned int _min_number,
                            unsigned int* _n1,
                            unsigned int* _n2,
                            unsigned int _return_fib_index);


/* Initializes the memory allocator and makes a portion of ’length’ bytes
   available. The allocator uses a ’basic_block_size’ as
   its minimal unit of allocation. The function returns the amount of
   memory made available to the allocator. If an error occurred,
   it returns 0. */
unsigned int init_allocator(unsigned int basic_block_size, unsigned int length);


/* release_allocator() returns any allocated memory to the operating system.
   After this function is called, any allocation fails. */
int release_allocator();


/* Allocate length number of bytes of free memory and returns the
   address of the allocated portion. Returns 0 when out of memory. */
Addr my_malloc(unsigned int length);


/* Frees the section of physical memory previously allocated
   using ’my_malloc’. Returns 0 if everything ok. */
int my_free(Addr _addr);


/* Output free_list data */
void show_free_list();

#endif /* defined(__Memory_Allocator__C___my_malloc__) */
