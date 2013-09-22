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

