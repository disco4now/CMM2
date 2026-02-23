#include "rotate.h"
#include "array_utility.h"
#include "bilinear_interpolation.h"
#include <stdio.h>
#include <math.h>
extern void CheckSDCard(void);
extern void routinechecks(int all);

unsigned char*** rotate (unsigned char*** input, int M_in, int N_in, float rotation_factor)
{
    // Prompt the user.

    // Prepare the output files.
    uint8_t*** output = alloc3df(3, M_in, N_in);

    // Define the center of the image.
    float vertical_center = floorf(M_in / 2);
    float horizontal_center = floorf(N_in / 2);
    if(fabsf((float)M_in/2.0-floorf(M_in / 2))<0.2)vertical_center-=0.5;
    if(fabsf((float)N_in/2.0-floorf(N_in / 2))<0.2)horizontal_center-=0.5;
    // Loop through each pixel of the new image, select the new vertical
    // and horizontal positions, and interpolate the image to make the change.
    int i, j, k;
    float angle = rotation_factor * (float) Pi / 180;
    float cosangle=cosf(angle), sinangle=sinf(angle);
    for (i = 0; i < M_in; ++i) {
    	routinechecks(1);
        for (j = 0; j < N_in; ++j) {
            //    // Figure out how rotated we want the image.
             float vertical_position = cosangle *
            		  ((float)i - vertical_center) + sinangle *  ((float)j - horizontal_center)
                + vertical_center;
            float horizontal_position = -sinangle *
            		 ((float)i - vertical_center) + cosangle *  ((float)j - horizontal_center)
                + horizontal_center;

            // Figure out the four locations (and then, four pixels)
            // that we must interpolate from the original image.
            int top = floor(vertical_position);
            int bottom = top + 1;
            int left = floor(horizontal_position);
            int right = left + 1;

            // Check if any of the four locations are invalid. If they are,
            // skip interpolating this pixel. Otherwise, interpolate the
            // pixel according to the dimensions set above and set the
            // resulting pixel.
            if (top >= 0 && bottom < M_in && left >= 0 && right < N_in ) {
                for (k = 0; k < 3; k++) {
                    float interpolated = bilinearly_interpolate(top, bottom,
                        left, right, horizontal_position, vertical_position,
                        input[k]);
                        output[k][i][j] = (uint8_t)interpolated;
                }
            }
        }
    }

    return output;
}


