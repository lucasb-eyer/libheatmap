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

#include <iostream>
#include <cmath>

#include "lodepng.h"
#include "heatmap.h"
#include "colorschemes/gray.h"

int main(int argc, char* argv[])
{
    if(argc != 2) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " RADIUS" << std::endl;
        std::cout << std::endl;
        std::cout << "  Will generate some stamps of given RADIUS and render" << std::endl;
        std::cout << "  them next to eachother into `stamps.png`." << std::endl;

        return 1;
    }

    const size_t r = atoi(argv[1]), d = 2*r+1, w = 5*d, h = d;
    heatmap_t* hm = heatmap_new(w, h);

    auto s_def = heatmap_stamp_gen(r);
    auto s_fat = heatmap_stamp_gen_nonlinear(r, [](float d){return d*d;});
    auto s_FAT = heatmap_stamp_gen_nonlinear(r, [](float d){return d*d*d*d;});
    auto s_pty = heatmap_stamp_gen_nonlinear(r, [](float d){return sqrtf(d);});
    auto s_PTY = heatmap_stamp_gen_nonlinear(r, [](float d){return sqrtf(sqrtf(sqrtf(d)));});

    heatmap_add_point_with_stamp(hm,     r, r, s_def);
    heatmap_add_point_with_stamp(hm,   d+r, r, s_fat);
    heatmap_add_point_with_stamp(hm, 2*d+r, r, s_FAT);
    heatmap_add_point_with_stamp(hm, 3*d+r, r, s_pty);
    heatmap_add_point_with_stamp(hm, 4*d+r, r, s_PTY);

    heatmap_stamp_free(s_def);
    heatmap_stamp_free(s_fat);
    heatmap_stamp_free(s_FAT);
    heatmap_stamp_free(s_pty);
    heatmap_stamp_free(s_PTY);

    std::vector<unsigned char> image(w*h*4);
    heatmap_render_to(hm, heatmap_cs_b2w_opaque, &image[0]);
    heatmap_free(hm);

    if(unsigned error = lodepng::encode("stamps.png", image, w, h)) {
        std::cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        return 1;
    }

    return 0;
}
