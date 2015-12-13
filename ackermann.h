/***********************************************************************************
 File: ackerman.h
 
 Authors: Professor R. Bettati
 Department of Computer Science
 Texas A&M University
 Date: 08/02/2009
 
 This file declares the function "ackermann(n,m)".
***********************************************************************************/

#ifndef __Memory_Allocator__C___ackerman__
#define __Memory_Allocator__C___ackerman__

/*--------------------------------------------------------------------------*/
/* MODULE ackerman */
/*--------------------------------------------------------------------------*/

extern void ackermann_main();
/* Asks user for parameters n and m and computes the result of the
 (highly recursive!) ackerman function. During every recursion step,
 it allocates and de-allocates a portion of memory with the use of the
 memory allocator defined in module "my_allocator.H".
 */


#endif /* defined(__Memory_Allocator__C___ackerman__) */
