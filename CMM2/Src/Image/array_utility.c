#include "array_utility.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "Memory.h"
#include "MMBasic.h"

unsigned char* alloc1df (int n)
{
//    int i;
    unsigned char* array;
    if ((array = (uint8_t*) GetMemory(n * sizeof(uint8_t))) == NULL) {
        error("Unable to allocate memory for 1D float array...\n");
        exit(0);
    }

//    for (i = 0; i < n; i++) {
//        array[i] = 0.0;
//    }

    return array;
}

unsigned char** alloc2df (int m, int n)
{
    int i;
    unsigned char** array;
    if ((array = (uint8_t **) GetMemory(m * sizeof(uint8_t*))) == NULL) {
        error("Unable to allocate memory for 2D float array...\n");
        exit(0);
    }

    for (i = 0; i < m; i++) {
        array[i] = alloc1df(n);
    }

    return array;
}

unsigned char*** alloc3df (int l, int m, int n)
{
    int i;
    uint8_t*** array;

    if ((array = (uint8_t ***) GetMemory(l * sizeof(uint8_t**))) == NULL) {
        error("Unable to allocate memory for 3D float array...\n");
        exit(0);
    }

    for (i = 0; i < l; i++) {
        array[i] = alloc2df(m,n);
    }

    return array;
}


void dealloc2df (uint8_t** array, int m, int n)
{
    int i;
    for (i = 0; i < m; i++) {
        FreeMemorySafe((void *)&array[i]);
    }

    FreeMemorySafe((void *)&array);
}

void dealloc3df (uint8_t*** array, int l, int m, int n)
{
    int i;
    for (i = 0; i < l; i++) {
        dealloc2df(array[i], m, n);
    }

    FreeMemorySafe((void *)&array);
}




