/*

This header file provides utility operations that allow for easy manipulation
of image arrays.

This header file is a rewritten file based off of a version given by Prof. Ruye
Wang of Harvey Mudd's Department of Engineering (2013).

*/

#ifndef ARRAY_UTILITY_H
#define ARRAY_UTILITY_H

#include <stdio.h>
#include <stdint.h>


unsigned char* alloc1df (int n);
unsigned char** alloc2df (int m, int n);
unsigned char*** alloc3df (int l, int m, int n);

void dealloc2df (uint8_t** array, int m, int n);
void dealloc3df (uint8_t*** array, int l, int m, int n);


// This function prints an end of file error message and exits.

#endif
