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

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
double gimme_time() {
    return omp_get_wtime();
}
#else
#include <time.h>
double gimme_time() {
    // Note that this way of timing doesn't have too high a resolution. I'm lazy.
    return static_cast<double>(clock())/static_cast<double>(CLOCKS_PER_SEC);
}
#endif

struct Timer {
    Timer() : _t0(gimme_time()) {}
    ~Timer() {
        double t1 = gimme_time();
        std::cout << "done in " << (t1-_t0)*1000.0 << "ms" << std::endl;
    }
    double _t0;
};

struct RepeatTimer {
    RepeatTimer(int count) : _t0(gimme_time()), _ts(count), _i(_ts.begin()) {}
    bool next() {
        *_i++ = gimme_time() - _t0;
        bool done = _i != _ts.end();
        _t0 = gimme_time();
        return done;
    }
    operator bool() const {
        return _i != _ts.end();
    }
    ~RepeatTimer() {
        double avg = std::accumulate(_ts.begin(), _ts.end(), 0.0) / static_cast<double>(_ts.size());

        // Figure out the median time.
        std::sort(_ts.begin(), _ts.end());
        double med = _ts[_ts.size()/2];
        std::cout << "done in a median of " << std::fixed << med * 1000.0 << "ms (avg is " << avg * 1000.0 << ")" << std::endl;
        std::cerr << "'medt': " << med << ", 'avgt': " << avg << "}," << std::endl;
    }
    double _t0;
    std::vector<double> _ts;
    std::vector<double>::iterator _i;
};

