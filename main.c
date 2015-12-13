/****************************************************************************
 File: main.c
 
 Author: Adrien Mombo-Caristan
 Department of Computer Science
 Texas A&M University
 Date initiated: 01/31/2014
 
 This file contains the main() function for demonstrating implementation of
 a fibonacci memory allocator.
****************************************************************************/


#include "my_malloc.h"
#include "ackermann.h"


int main(int argc, const char * argv[]) {
    
    /* Input parameters (basic block size, memory length) */
    
    /*init_allocator(256, 1048576);
    
    ackermann_main();
    
    release_allocator();*/
    
    /* Self Test */
    init_allocator(32, 95000);
    
    show_free_list();
    
    Addr allocation1 = my_malloc(45);
    printf("\n%d", (int)allocation1);
    show_free_list();
    
    Addr allocation2 = my_malloc(8672);
    printf("\n%d", (int)allocation2);
    show_free_list();
    
    Addr allocation3 = my_malloc(1249);
    printf("\n%d", (int)allocation3);
    show_free_list();
    
    Addr allocation4 = my_malloc(30000);
    printf("\n%d", (int)allocation3);
    show_free_list();
    
    printf("\n%d", my_free(allocation3));
    show_free_list();
    
    printf("\n%d", my_free(allocation1));
    show_free_list();
    
    printf("\n%d", my_free(allocation4));
    show_free_list();
    
    printf("\n%d", my_free(allocation2));
    show_free_list();
    
    release_allocator();
    
    return 0;
}
