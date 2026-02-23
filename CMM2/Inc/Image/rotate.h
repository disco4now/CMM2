#ifndef ROTATE_H
#define ROTATE_H

#define Pi 3.141592653589

#include "bilinear_interpolation.h"

unsigned char*** rotate (unsigned char*** input, int M_in, int N_in, float rotation_factor);

#endif
