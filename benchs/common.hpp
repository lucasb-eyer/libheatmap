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

#pragma once

#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include <memory>

#include "benchs/timing.hpp"
#include "heatmap.h"

// Makes unique_ptr "understand" how to delete heatmap_t
// without tracking any additional state.
namespace std {
    template<>
    struct default_delete<heatmap_t> {
        void operator()(heatmap_t* p) { heatmap_free(p); }
    };
    template<>
    struct default_delete<heatmap_stamp_t> {
        void operator()(heatmap_stamp_t* p) { heatmap_stamp_free(p); }
    };
}

inline std::vector<unsigned> genpoints(size_t npoints, unsigned maxval)
{
    std::vector<unsigned> points(npoints*2);

    std::random_device rd;
    std::mt19937 prng(rd());
    std::uniform_int_distribution<> dist(0, maxval);

    std::cout << "Generating " << npoints << " random points... " << std::flush;
    { Timer t;
        std::generate(points.begin(), points.end(), std::bind(dist, prng));
    }

    return points;
}

