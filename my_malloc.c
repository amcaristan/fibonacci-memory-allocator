/***********************************************************************************
 File: my_malloc.c
 
 Author: Adrien Mombo-Caristan
 Department of Computer Science
 Texas A&M University
 Date: 01/31/2014
 
 This file contains the implementations for the my_malloc module as well as List
 and Header for doubly-linked list implementation.
***********************************************************************************/


#include <stdlib.h>
#include "my_malloc.h"


static void* allocated_memory_front = NULL;
static void* allocated_memory_back = NULL;
static unsigned int final_allocation_size = 0;
static unsigned int final_basic_block_size = 0;
static unsigned int free_list_size = 0;
static Header** free_list = NULL;
static unsigned short int memory_valid = 0;


unsigned int find_fibonacci(unsigned int _min_number,
                            unsigned int* _n1,
                            unsigned int* _n2,
                            unsigned int _return_fib_index) {
    unsigned int fibonacci_index = 0;
    unsigned int fibonacci_num = 0;
    unsigned int n1 = 0;
    unsigned int n2 = 1;
    
    if (_n1 ==  NULL && _n2 == NULL) {
        do {
            fibonacci_num = n1 + n2;
            n1 = n2;
            n2 = fibonacci_num;
            ++fibonacci_index;
        } while (fibonacci_num < _min_number);
    } else {
        *_n1 = 0;
        *_n2 = 1;
        
        do {
            fibonacci_num = *_n1 + *_n2;
            *_n1 = *_n2;
            *_n2 = fibonacci_num;
            ++fibonacci_index;
        } while (fibonacci_num < _min_number);
        
        *_n2 = *_n1;
        *_n1 = fibonacci_num - *_n2;
    }
    
    if (_return_fib_index) {
        return fibonacci_index;
    } else {
        return fibonacci_num;
    }
}


/* Add block pointed to by _hdr to the appropriate free_list index */
static void make_available(Header* _hdr) {
    Header* block = NULL;
    unsigned int free_list_index = find_fibonacci(_hdr->block_count, NULL, NULL, 1);
    
    if (free_list[ free_list_index ] == NULL) {
        free_list[ free_list_index ] = _hdr;
        _hdr->prev = _hdr->next = NULL;
    } else {
        block = free_list[ free_list_index ];
        
        while (block->next != NULL) {
            block = block->next;
        }
        
        block->next = _hdr;
        _hdr->prev = block;
        _hdr->next = NULL;
    }
    
    _hdr->is_free = 'Y';
}


/* Remove block pointed to by _hdr from associated free_list index. Should always
   be used in conjunction with add_to_allocation_queue() */
static void make_unavailable(Header* _hdr) {
    unsigned int free_list_index = find_fibonacci(_hdr->block_count, NULL, NULL, 1);
    
    if (_hdr->next == NULL) {
        if (_hdr->prev == NULL) {
            free_list[ free_list_index ] = NULL;
        } else {
            _hdr->prev->next = NULL;
        }
    } else {
        if (_hdr->prev == NULL) {
            free_list[ free_list_index ] = _hdr->next;
            _hdr->next->prev = NULL;
        } else {
            _hdr->prev->next = _hdr->next;
            _hdr->next->prev = _hdr->prev;
        }
    }
    
    _hdr->next = _hdr->prev = NULL;
    
    _hdr->is_free = 'N';
}


/* Add block pointed to by _hdr to allocation queue, a.k.a, free_list[0] */
static void add_to_allocation_queue(Header* _hdr) {
    
    if (free_list[0] == NULL) {
        free_list[0] = _hdr;
        _hdr->prev = _hdr->next = NULL;
    } else {
        Header* block = free_list[0];
        
        while (block->next != NULL) {
            block = block->next;
        }
        
        block->next = _hdr;
        _hdr->prev = block;
        _hdr->next = NULL;
    }
}


/* Remove block pointed to by _hdr from allocation queue, a.k.a, free_list[0]. 
   Should always be used in conjunction with make_available() */
static void remove_from_allocation_queue(Header* _hdr) {
    if (_hdr->prev == NULL) {
        if (_hdr->next == NULL) {
            free_list[ 0 ] = NULL;
        } else {
            free_list[ 0 ] = _hdr->next;
            _hdr->next->prev = NULL;
        }
    } else {
        if (_hdr->next == NULL) {
            _hdr->prev->next = NULL;
        } else {
            _hdr->prev->next = _hdr->next;
            _hdr->next->prev = _hdr->prev;
        }
    }
    
    _hdr->prev = _hdr->next = NULL;
}


/* Split block pointed to by _hdr into constituent buddy blocks and depending on
   _get_right_child set _child pointer to the newly created right/left child of split. */
static void split(Header* _hdr, Header* _child, unsigned short int _get_right_child) {
    unsigned int n1 = 0;
    unsigned int n2 = 0;
    unsigned int free_list_index = find_fibonacci(_hdr->block_count, &n1, &n2, 1);
    
    Header* parent = free_list[ free_list_index ];
    make_unavailable(parent);
    
    void* left_child = parent;
    void* right_child = (char*)left_child + (n2 * final_basic_block_size);
    
    /* These parameters need to be set before any others so that proper values are 
       not overwritten */
    ((Header*)right_child)->inherit = parent->inherit;
    ((Header*)left_child)->inherit = parent->child;
    
    /* Left child will always be the larger block, and its inheritance bit is set
       to the child, i.e. left/right, bit of the parent */
    ((Header*)left_child)->block_count = n2;
    ((Header*)left_child)->child = 'L';
    ((Header*)left_child)->header_ident = HEADER_IDENT;
    
    make_available((Header*)left_child);
    
    /* Right child's inheritance bit is set to the inheritance bit of the parent */
    ((Header*)right_child)->block_count = n1;
    ((Header*)right_child)->child = 'R';
    ((Header*)right_child)->header_ident = HEADER_IDENT;
    
    make_available((Header*)right_child);
    
    _child = (Header*)(_get_right_child ? right_child : left_child);
}


/* Attempt to combine the block pointed to by _hdr with its respective buddy, 
   returns 1 if another immediate coalesce is possible, 0 otherwise. */
static int coalesce(Header** _hdr) {
    if ((*_hdr)->child == 'L') {
        unsigned int n1 = 0;
        unsigned int n2 = 0;
        void* right_child = (char*)(*_hdr) + ((*_hdr)->block_count * final_basic_block_size);
        
        if (((Header*)right_child)->header_ident != HEADER_IDENT) {
            return 0;
        }
        
        find_fibonacci((*_hdr)->block_count, &n1, &n2, 0);
        
        if ((((Header*)right_child)->block_count == n2) && ((Header*)right_child)->is_free == 'Y') {
            make_unavailable(*_hdr);
            make_unavailable((Header*)right_child);
            
            (*_hdr)->block_count += ((Header*)right_child)->block_count;
            (*_hdr)->child = (*_hdr)->inherit;
            (*_hdr)->header_ident = HEADER_IDENT;
            (*_hdr)->inherit = ((Header*)right_child)->inherit;
            
            make_available(*_hdr);
        } else {
            return 0;
        }
        
    } else if ((*_hdr)->child == 'R') {
        unsigned int n1 = 0;
        unsigned int n2 = 0;
        
        find_fibonacci((*_hdr)->block_count, &n1, &n2, 0);
        
        void* left_child = (char*)(*_hdr) - (((*_hdr)->block_count + n2) * final_basic_block_size);
        
        if (((Header*)left_child)->header_ident != HEADER_IDENT) {
            return 0;
        }
        
        if ((((Header*)left_child)->block_count == ((*_hdr)->block_count + n2)) &&
                ((Header*)left_child)->is_free == 'Y') {
            make_unavailable(*_hdr);
            make_unavailable((Header*)left_child);
            
            ((Header*)left_child)->block_count += (*_hdr)->block_count;
            ((Header*)left_child)->child = ((Header*)left_child)->inherit;
            ((Header*)left_child)->header_ident = HEADER_IDENT;
            ((Header*)left_child)->inherit = (*_hdr)->inherit;
            
            *_hdr = ((Header*)left_child);
            
            make_available(*_hdr);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
    
    return 1;
}


unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length) {
    unsigned int allocation_size = 0;
    unsigned int number_of_blocks = 0;
    unsigned int initial_block_amt = 0;
    
    /* basic_block_size should not be smaller than sizeof(Header) */
    if (_basic_block_size < sizeof(Header)) {
        final_basic_block_size = sizeof(Header);
    } else {
        final_basic_block_size = _basic_block_size;
    }
    
    /* Make sure there is enough space for the memory management 'Header' */
    allocation_size = _length + sizeof(Header);
    if (allocation_size <= final_basic_block_size) {
        allocation_size += final_basic_block_size - allocation_size;
    } else {
        allocation_size += final_basic_block_size -
                            (allocation_size % final_basic_block_size);
    }
    
    /* Make allocation a multiple of a basic_block_size */
    number_of_blocks = allocation_size / final_basic_block_size;
    final_allocation_size = final_basic_block_size *
                                find_fibonacci(number_of_blocks, NULL, NULL, 0);
    number_of_blocks = final_allocation_size / final_basic_block_size;
    
    allocated_memory_front = malloc(final_allocation_size);
    allocated_memory_back = (char*)allocated_memory_front + final_allocation_size;
    
    printf("\nRequested memory: %i bytes\nAllocated memory: %u bytes",
           _length, final_allocation_size);
    printf("\nAvailable memory: %lu bytes",
           final_allocation_size - sizeof(Header));
    
    printf("\n\n#Blocks: %i\nFib Index: %u", number_of_blocks,
           find_fibonacci(number_of_blocks, NULL, NULL, 1));
    
    /* Intializing freeList, add 1 extra list element to hold allocated 
       blocks: free_list[0] */
    free_list_size = find_fibonacci(number_of_blocks, NULL, NULL, 1) + 1;
    initial_block_amt = find_fibonacci(number_of_blocks, NULL, NULL, 0);
    
    printf("\nfree_list_size: %u", free_list_size);
    
    free_list = (Header**) malloc(free_list_size * sizeof(Header*));
    
    free_list[ free_list_size - 1 ] = (Header*) allocated_memory_front;
    free_list[ free_list_size - 1 ]->prev = NULL;
    free_list[ free_list_size - 1 ]->next = NULL;
    free_list[ free_list_size - 1 ]->header_ident = HEADER_IDENT;
    free_list[ free_list_size - 1 ]->block_count = initial_block_amt;
    free_list[ free_list_size - 1 ]->child = '-';
    free_list[ free_list_size - 1 ]->inherit = '-';
    free_list[ free_list_size - 1 ]->is_free = 'Y';
    
    for (int i = free_list_size - 2; i >= 0; i--) {
        free_list[i] = NULL;
    }
    
    memory_valid = 1; /* Allow allocations */
    
    printf("\n\n%i bytes have been allocated for use, and free_list initialized.",
           final_allocation_size);
    
    printf("\nThe free-list has a pointer to %u free allocated bytes",
           free_list[ free_list_size - 1 ]->block_count *
                                                final_basic_block_size);
    
    printf("\nMemory and allocator initialized successfully.\n\n");
    
    return final_allocation_size;
    
error:
    return -1;
}


int release_allocator() {
    free(free_list);
    free_list_size = 0;
    free(allocated_memory_front);
    allocated_memory_front = allocated_memory_back = NULL;
    final_allocation_size = 0;
    final_basic_block_size = 0;
    
    memory_valid = 0; /* Disallow allocations */
    
    printf("\nMemory released and allocator uninitialized successfully.");
    
    return 0;
    
error:
    printf("\n!--- FAIL (release_alocator): Problem releasing memory/allocator. ---!\n");
    return -1;
}


extern Addr my_malloc(unsigned int _length) {
    if (!memory_valid) {
        return 0;
    }
    
    Addr return_address = NULL;
    Header* hdr = NULL;
    unsigned int blocks_to_allocate = 0;
    unsigned int free_list_index = 0;
    int temp = 0;
    
    if (_length < (final_basic_block_size - sizeof(Header))) { /* 1 block needed */
        blocks_to_allocate = 1;
    } else if (_length < final_basic_block_size) {  /* Need for 2 blocks */
        blocks_to_allocate = 2;
    } else {    /* Need for more than 2 blocks */
        blocks_to_allocate = (_length + sizeof(Header)) / final_basic_block_size;
        
        if ((_length + sizeof(Header)) % final_basic_block_size > 0) {
            ++blocks_to_allocate;
        }
        
        blocks_to_allocate = find_fibonacci(blocks_to_allocate, NULL, NULL, 0);
    }
    
    free_list_index = find_fibonacci(blocks_to_allocate, NULL, NULL, 1);
    temp = (int)free_list_index;
    
    /* Locate smallest, appropriate, and available block of memory to serve request */
    while (temp < free_list_size) {
        if ( free_list[ temp ] == NULL ) {
            ++temp;
        } else {
            break;
        }
    }
    
    if (temp >= free_list_size) { /* Not enough memory available */
        printf("\n!--- FAIL (my_malloc): Not enough memory available. ---!\n");
        return 0;
    }
    
    temp = temp - free_list_index;
    
    switch (temp) {
        case 0: /* Block of appropriate size available */
            if (free_list[ free_list_index ]->header_ident != HEADER_IDENT) {
                printf("\n!--- FAIL (my_malloc): Invalid block access. ---!\n");
                return 0;
            }
            
            return_address = (char*)free_list[ free_list_index ] + sizeof(Header);
            
            /* Remove block from current position and place block at end of
               free_list[0] queue */
            hdr = free_list[ free_list_index ];
            make_unavailable(hdr);
            add_to_allocation_queue(hdr);
            
            /* For testing purposes */
            //show_free_list();
            
            return return_address;
            
            break;
            
        default: /* Split larger block into appropriately sized one to serve request */
        {
            div_t division = div(temp, 2);
            Header* child = NULL;
            unsigned short int quit_loop = 0;
            
            while ((free_list[ free_list_index ] == NULL) && !quit_loop) {
                if (division.quot >= 1) {
                    split(free_list[ free_list_index + temp ], child, 1);
                    temp -= 2;
                } /*else if (division.quot == 1) {
                    if (division.rem == 1) {
                        split(free_list[ free_list_index + temp ], child, 0);
                        break;
                    } else {
                        split(free_list[ free_list_index + temp ], child, 1);
                        break;
                    }
                }*/ else if ((division.quot) < 1) {
                    switch (division.rem) {
                        case 0:
                            split(free_list[ free_list_index + temp ], child, 1);
                            quit_loop = 1;
                            break;
                        case 1:
                            split(free_list[ free_list_index + temp ], child, 0);
                            quit_loop = 1;
                            break;
                        default:
                            printf("\n!--- FAIL (my_malloc): Something went very wrong here. ---!\n");
                            return 0;
                            break;
                    }
                }
                
                --division.quot;
            }
            
            return_address = (char*)free_list[ free_list_index ] + sizeof(Header);
            
            /* Remove block from current position and place block at end of
               free_list[0] queue */
            hdr = free_list[ free_list_index ];
            make_unavailable(hdr);
            add_to_allocation_queue(hdr);
            
            /* For testing purposes */
            //show_free_list();
            
            return return_address;

            break;
        }
    }
  
error:
    return 0;
}


int my_free(Addr _addr) {
    Header* hdr = (Header*)((char*)_addr - sizeof(Header));
    
    remove_from_allocation_queue(hdr);
    make_available(hdr);
    
    while( coalesce(&hdr) );
    
    /* For testing purposes */
    //show_free_list();
    
    return 0;
    
error:
    return 1;
}


void show_free_list() {
    printf("\n\n");
    for (int i = 1; i <= free_list_size; i++) {
        
        void* hdr = free_list[ free_list_size - i ];
        
        if (hdr != NULL) {
            
            printf("[%i]: ", free_list_size - i);
            
            do {
                if (((Header*)hdr)->header_ident == HEADER_IDENT) {
                    printf("%d(%c) -> ",((Header*)hdr)->block_count * final_basic_block_size,
                           ((Header*)hdr)->child);
                }
                
                hdr = ((Header*)hdr)->next;

            } while (hdr != NULL);
            
            printf("NULL\n");
            
        } else {
            
            printf("[%i]: Empty\n", free_list_size - i);
            
        }
    }
}



