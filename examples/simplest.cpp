/* heatmap - High performance heatmap creation in C.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Lucas Beyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <random>
#include <vector>
#include <iostream>

#include "lodepng.h"
#include "heatmap.h"

int main()
{
    static const size_t w = 256, h = 512, npoints = 1000;

    // Create the heatmap object with the given dimensions (in pixel).
    heatmap_t* hm = heatmap_new(w, h);

    // This creates two normal random distributions which will give us random points.
    std::random_device rd;
    std::mt19937 prng(rd());
    std::normal_distribution<float> x_distr(0.5f*w, 0.5f/3.0f*w), y_distr(0.5f*h, 0.25f*h);

    // Add a bunch of random points to the heatmap now.
    for(unsigned i = 0 ; i < npoints ; ++i) {
        heatmap_add_point(hm, x_distr(prng), y_distr(prng));
    }

    // This creates an image out of the heatmap.
    // `image` now contains the image data in 32-bit RGBA.
    std::vector<unsigned char> image(w*h*4);
    heatmap_render_default_to(hm, &image[0]);

    // Now that we've got a finished heatmap picture, we don't need the map anymore.
    heatmap_free(hm);

    // Finally, we use the fantastic lodepng library to save it as an image.
    if(unsigned error = lodepng::encode("heatmap.png", image, w, h)) {
        std::cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        return 1;
    }

    return 0;
}
