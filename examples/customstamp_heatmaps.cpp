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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <cmath>
#include <array>

#include "lodepng.h"
#include "heatmap.h"
#include "colorschemes/gray.h"

using namespace std;

int main(int argc, char* argv[])
{
    if(argc != 2) {
        cerr << "Invalid number of arguments!" << endl;
        cout << "Usage:" << endl;
        cout << "  " << argv[0] << " RADIUS" << endl;
        cout << endl;
        cout << "  Will generate some stamps of given RADIUS and render" << endl;
        cout << "  them next to eachother into `stamps.png`." << endl;

        return 1;
    }

    const size_t r = atoi(argv[1]), d = 2*r+1;

    vector<float> rhomboidal(d*d, 0.0f);
    for(long y = 0 ; y < (int)d ; ++y) {
        for(long x = 0 ; x < (int)d ; ++x) {
            rhomboidal[y*d+x] = 1.0f - min(1.0f, (float)(abs(x-(long)r)+abs(y-(long)r))/(r+1));
        }
    }

    heatmap_stamp_t* stamps[] = {
        heatmap_stamp_gen(r),
        heatmap_stamp_gen_nonlinear(r, [](float d){return d*d*d*d*d*d;}),
        heatmap_stamp_gen_nonlinear(r, [](float d){return sqrtf(d);}),
        heatmap_stamp_gen_nonlinear(r, [](float d){return sqrtf(sqrtf(sqrtf(d)));}),
        heatmap_stamp_load(d, d, &rhomboidal[0]),
    };

    const size_t nstamps = sizeof(stamps)/sizeof(stamps[0]);

    // First, draw a picture containing all of the stamps.
    {
        const size_t w = nstamps*d, h = d;
        heatmap_t* hm = heatmap_new(w, h);

        for(size_t i = 0 ; i < nstamps ; ++i)
            heatmap_add_point_with_stamp(hm, i*d+r, r, stamps[i]);

        vector<unsigned char> image(w*h*4);
        heatmap_render_to(hm, heatmap_cs_b2w_opaque, &image[0]);
        heatmap_free(hm);

        if(unsigned error = lodepng::encode("stamps.png", image, w, h)) {
            std::cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
            return 1;
        }
    }

    // Then, draw actual heatmaps using these stamps.
    // We draw each heatmap individually because we want each of them to be
    // normalized independently.
    {
        const size_t w = d*20, h = w, npoints = 1000;
        array<heatmap_t*, nstamps> hms;
        for(auto& hm : hms) {
            hm = heatmap_new(w, h);
        }

        // This creates two normal random distributions...
        random_device rd;
        mt19937 prng(rd());
        normal_distribution<float> x_distr(0.5f*w, 0.5f/3.0f*w), y_distr(0.5f*h, 0.25f*h);
        // ...which are then used to generate the random points.
        vector<pair<float,float>> points(npoints);
        generate(points.begin(), points.end(), [&](){return make_pair(x_distr(prng), y_distr(prng));});

        char filename[] = "stamps_heatmapA.png";
        for(size_t i = 0 ; i < nstamps ; ++i) {
            auto* hm = hms[i];

            for(auto point : points) {
                heatmap_add_point_with_stamp(hm, point.first, point.second, stamps[i]);
            }

            vector<unsigned char> image(w*h*4);
            heatmap_render_default_to(hm, &image[0]);
            heatmap_free(hm);

            if(unsigned error = lodepng::encode(filename, image, w, h)) {
                cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << endl;
                return 1;
            }

            // INSANE hack :)
            filename[14]++;
        }
    }

    for(auto stamp : stamps)
        heatmap_stamp_free(stamp);

    return 0;
}
