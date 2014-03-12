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

// Test the speed of calling the heatmap point adding function once for
// every single point (a la glVertex3f) vs. calling one function which
// adds a whole buffer of points (a la glVertexPointer). Basically, time
// the function call overhead.

#include "benchs/common.hpp"

static const size_t MAPSIZE_MIN = 128;
static const size_t MAPSIZE_MAX = 16384; // That's 1 gig of ram already
static const size_t NPOINTS = 1000;
static const size_t STAMP = 128;

int main(/* int argc, char *argv[] */)
{
    // We'll do something funky with ret in order to avoid optimizing
    // whole code-blocks away.
    int ret = 0;

    std::unique_ptr<heatmap_stamp_t> stamp(heatmap_stamp_gen(STAMP));

    std::cerr << "[" << std::endl;
    for(size_t mapsize = MAPSIZE_MIN ; mapsize <= MAPSIZE_MAX ; mapsize *= 2) {
        // All of this is preparing the heatmap to be rendered.
        std::unique_ptr<heatmap_t> hm(heatmap_new(mapsize, mapsize));
        auto points = genpoints(NPOINTS, mapsize);

        for(size_t i = 0 ; i < NPOINTS ; ++i) {
            heatmap_add_point_with_stamp(hm.get(), points[2*i], points[2*i+1], stamp.get());
        }
        std::vector<unsigned char> imgbuf(mapsize*mapsize*4);

        // Finally, we can render it!
        std::cerr << "{'mapsize': " << mapsize << ", 'saturation': false, ";
        std::cout << "Rendering a " << mapsize << "² map without saturation... " << std::flush;
        for(RepeatTimer t(5) ; t ; t.next()) {
            heatmap_render_to(hm.get(), heatmap_cs_default, &imgbuf[0]);
        }
        ret += imgbuf[0];

        std::cerr << "{'mapsize': " << mapsize << ", 'saturation': true, ";
        std::cout << "Rendering a " << mapsize << "² map with saturation... " << std::flush;
        for(RepeatTimer t(5) ; t ; t.next()) {
            heatmap_render_saturated_to(hm.get(), heatmap_cs_default, 0.5f, &imgbuf[0]);
        }
        ret += imgbuf[0];
    }
    std::cerr << "]" << std::endl;

    return ret;
}

