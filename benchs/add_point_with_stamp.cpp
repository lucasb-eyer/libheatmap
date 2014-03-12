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

static const size_t NPOINTS_MIN = 1;
static const size_t NPOINTS_MAX = 1000*1000;
static const size_t MAPSIZE = 15360;
static const size_t STAMP_MIN = 1;
static const size_t STAMP_MAX = 512;

int main(/* int argc, char *argv[] */)
{
    // We'll do something funky with ret in order to avoid optimizing
    // whole code-blocks away.
    int ret = 0;
    auto points = genpoints(NPOINTS_MAX, MAPSIZE);

    std::cerr << "[" << std::endl;
    for(size_t stampsize = STAMP_MIN ; stampsize <= STAMP_MAX ; stampsize *= 2) {
        std::unique_ptr<heatmap_stamp_t> stamp(heatmap_stamp_gen(stampsize));
        for(size_t npoints = NPOINTS_MIN ; npoints <= NPOINTS_MAX ; npoints *= 10) {
            std::unique_ptr<heatmap_t> hm(heatmap_new(MAPSIZE, MAPSIZE));
            std::cerr << "{'npoints': " << npoints << ", 'size': " << stampsize << ", ";
            std::cout << "Adding " << npoints << " points of size " << stampsize << " one after another... " << std::flush;
            for(RepeatTimer t(5) ; t ; t.next()) {
                for(size_t i = 0 ; i < npoints ; ++i) {
                    heatmap_add_point_with_stamp(hm.get(), points[2*i], points[2*i+1], stamp.get());
                }
            }

            ret += hm->buf[0] > 0.0f;
        }
    }

    return ret;
}

