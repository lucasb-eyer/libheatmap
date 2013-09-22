#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "lodepng.h"
#include "heatmap.h"

int main()
{
    static const size_t w = 256, h = 512, npoints = 1000;
    unsigned char image[256*512*4];
    unsigned i;

    /* Create the heatmap object with the given dimensions (in pixel). */
    heatmap_t* hm = heatmap_new(w, h);

    srand(time(NULL));

    /* Add a bunch of random points to the heatmap now. */
    for(i = 0 ; i < npoints ; ++i) {
        /* Fake a normal distribution. */
        unsigned x = rand() % w/3 + rand() % w/3 + rand() % w/3;
        unsigned y = rand() % h/3 + rand() % h/3 + rand() % h/3;
        heatmap_add_point(hm, x, y);
    }

    /* This creates an image out of the heatmap.
     * `image` now contains the image data in 32-bit RGBA.
     */
    heatmap_render_default_to(hm, image);

    /* Now that we've got a finished heatmap picture,
     * we don't need the map anymore.
     */
    heatmap_free(hm);

    /* Finally, we use the fantastic lodepng library to save it as an image. */
    {
        unsigned error = lodepng_encode32_file("heatmap.png", image, w, h);
        if(error)
            fprintf(stderr, "Error (%u) creating PNG file: %s\n",
                    error, lodepng_error_text(error));
    }

    return 0;
}
