#include "resize.h"
#include "bilinear_interpolation.h"
#include "array_utility.h"
#include <math.h>
extern void CheckSDCard(void);
extern void routinechecks(int all);

unsigned char*** resize (unsigned char*** input, int M_in, int N_in, int output_height,
    int output_width)
{
    // Prompt the user.
    // Figure out the dimensions of the new, resized image and define the
    // new array.
    int M_out_resized = output_height ;
    int N_out_resized = output_width ;
    unsigned char*** output = alloc3df(3, M_out_resized, N_out_resized);

    // Loop through each pixel of the new image and interpolate the pixels
    // based on what's in the input image.
    int i, j, k;
    for (i = 0; i < M_out_resized; ++i) {
    	routinechecks(1);
        for (j = 0; j < N_out_resized; ++j) {
            // Figure out how far down or across (in percentage) we are with
            // respect to the original image.
            float vertical_position = i * ( (float) M_in / M_out_resized);
            float horizontal_position = j * ( (float) N_in / N_out_resized);

            // Figure out the four locations (and then, four pixels)
            // that we must interpolate from the original image.
            int top = floorf(vertical_position);
            int bottom = top + 1;
            int left = floorf(horizontal_position);
            int right = left + 1;

            // Check if any of the four locations are invalid. If they are,
            // simply access the valid adjacent pixel.
            if (bottom >= M_in) {
                bottom = top;
            }
            if (right >= N_in) {
                right = left;
            }

            // Interpolate the pixel according to the dimensions
            // set above and set the resulting pixel. Do so for each color.
            for (k = 0; k < 3; k++) {
            	float interpolated = bilinearly_interpolate(top, bottom, left,
                    right, horizontal_position, vertical_position, input[k]);
                output[k][i][j] = (unsigned char)interpolated;
            }

        }
    }

    return output;
}
