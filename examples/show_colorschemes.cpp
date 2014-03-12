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
#include "colorschemes/Blues.h"
#include "colorschemes/BrBG.h"
#include "colorschemes/BuGn.h"
#include "colorschemes/BuPu.h"
#include "colorschemes/GnBu.h"
#include "colorschemes/Greens.h"
#include "colorschemes/Greys.h"
#include "colorschemes/Oranges.h"
#include "colorschemes/OrRd.h"
#include "colorschemes/PiYG.h"
#include "colorschemes/PRGn.h"
#include "colorschemes/PuBuGn.h"
#include "colorschemes/PuBu.h"
#include "colorschemes/PuOr.h"
#include "colorschemes/PuRd.h"
#include "colorschemes/Purples.h"
#include "colorschemes/RdBu.h"
#include "colorschemes/RdGy.h"
#include "colorschemes/RdPu.h"
#include "colorschemes/RdYlBu.h"
#include "colorschemes/RdYlGn.h"
#include "colorschemes/Reds.h"
#include "colorschemes/Spectral.h"
#include "colorschemes/YlGnBu.h"
#include "colorschemes/YlGn.h"
#include "colorschemes/YlOrBr.h"
#include "colorschemes/YlOrRd.h"

using namespace std;

int main(int argc, char* argv[])
{
    if(argc < 3 || 4 < argc ) {
        cerr << "Invalid number of arguments!" << endl;
        cout << "Usage:" << endl;
        cout << "  " << argv[0] << " WIDTH HEIGHT [RADIUS]" << endl;

        return 1;
    }

    const size_t w = atoi(argv[1]), h = atoi(argv[2]), r = (argc == 4 ? atoi(argv[3]) : min(w,h)/20);
    const size_t npoints = 1000;

    pair<const heatmap_colorscheme_t*, const char*> schemes[] = {
        make_pair(heatmap_cs_b2w,       "b2w.png"),
        make_pair(heatmap_cs_b2w_opaque,"b2w_opaque.png"),
        make_pair(heatmap_cs_w2b,       "w2b.png"),
        make_pair(heatmap_cs_w2b_opaque,"w2b_opaque.png"),
        make_pair(heatmap_cs_Blues_discrete, "Blues_discrete.png"),
        make_pair(heatmap_cs_Blues_soft,     "Blues_soft.png"),
        make_pair(heatmap_cs_Blues_mixed,    "Blues_mixed.png"),
        make_pair(heatmap_cs_Blues_mixed_exp,"Blues_mixed_exp.png"),
        make_pair(heatmap_cs_BrBG_discrete, "BrBG_discrete.png"),
        make_pair(heatmap_cs_BrBG_soft,     "BrBG_soft.png"),
        make_pair(heatmap_cs_BrBG_mixed,    "BrBG_mixed.png"),
        make_pair(heatmap_cs_BrBG_mixed_exp,"BrBG_mixed_exp.png"),
        make_pair(heatmap_cs_BuGn_discrete, "BuGn_discrete.png"),
        make_pair(heatmap_cs_BuGn_soft,     "BuGn_soft.png"),
        make_pair(heatmap_cs_BuGn_mixed,    "BuGn_mixed.png"),
        make_pair(heatmap_cs_BuGn_mixed_exp,"BuGn_mixed_exp.png"),
        make_pair(heatmap_cs_BuPu_discrete, "BuPu_discrete.png"),
        make_pair(heatmap_cs_BuPu_soft,     "BuPu_soft.png"),
        make_pair(heatmap_cs_BuPu_mixed,    "BuPu_mixed.png"),
        make_pair(heatmap_cs_BuPu_mixed_exp,"BuPu_mixed_exp.png"),
        make_pair(heatmap_cs_GnBu_discrete, "GnBu_discrete.png"),
        make_pair(heatmap_cs_GnBu_soft,     "GnBu_soft.png"),
        make_pair(heatmap_cs_GnBu_mixed,    "GnBu_mixed.png"),
        make_pair(heatmap_cs_GnBu_mixed_exp,"GnBu_mixed_exp.png"),
        make_pair(heatmap_cs_Greens_discrete, "Greens_discrete.png"),
        make_pair(heatmap_cs_Greens_soft,     "Greens_soft.png"),
        make_pair(heatmap_cs_Greens_mixed,    "Greens_mixed.png"),
        make_pair(heatmap_cs_Greens_mixed_exp,"Greens_mixed_exp.png"),
        make_pair(heatmap_cs_Greys_discrete, "Greys_discrete.png"),
        make_pair(heatmap_cs_Greys_soft,     "Greys_soft.png"),
        make_pair(heatmap_cs_Greys_mixed,    "Greys_mixed.png"),
        make_pair(heatmap_cs_Greys_mixed_exp,"Greys_mixed_exp.png"),
        make_pair(heatmap_cs_Oranges_discrete, "Oranges_discrete.png"),
        make_pair(heatmap_cs_Oranges_soft,     "Oranges_soft.png"),
        make_pair(heatmap_cs_Oranges_mixed,    "Oranges_mixed.png"),
        make_pair(heatmap_cs_Oranges_mixed_exp,"Oranges_mixed_exp.png"),
        make_pair(heatmap_cs_OrRd_discrete, "OrRd_discrete.png"),
        make_pair(heatmap_cs_OrRd_soft,     "OrRd_soft.png"),
        make_pair(heatmap_cs_OrRd_mixed,    "OrRd_mixed.png"),
        make_pair(heatmap_cs_OrRd_mixed_exp,"OrRd_mixed_exp.png"),
        make_pair(heatmap_cs_PiYG_discrete, "PiYG_discrete.png"),
        make_pair(heatmap_cs_PiYG_soft,     "PiYG_soft.png"),
        make_pair(heatmap_cs_PiYG_mixed,    "PiYG_mixed.png"),
        make_pair(heatmap_cs_PiYG_mixed_exp,"PiYG_mixed_exp.png"),
        make_pair(heatmap_cs_PRGn_discrete, "PRGn_discrete.png"),
        make_pair(heatmap_cs_PRGn_soft,     "PRGn_soft.png"),
        make_pair(heatmap_cs_PRGn_mixed,    "PRGn_mixed.png"),
        make_pair(heatmap_cs_PRGn_mixed_exp,"PRGn_mixed_exp.png"),
        make_pair(heatmap_cs_PuBuGn_discrete, "PuBuGn_discrete.png"),
        make_pair(heatmap_cs_PuBuGn_soft,     "PuBuGn_soft.png"),
        make_pair(heatmap_cs_PuBuGn_mixed,    "PuBuGn_mixed.png"),
        make_pair(heatmap_cs_PuBuGn_mixed_exp,"PuBuGn_mixed_exp.png"),
        make_pair(heatmap_cs_PuBu_discrete, "PuBu_discrete.png"),
        make_pair(heatmap_cs_PuBu_soft,     "PuBu_soft.png"),
        make_pair(heatmap_cs_PuBu_mixed,    "PuBu_mixed.png"),
        make_pair(heatmap_cs_PuBu_mixed_exp,"PuBu_mixed_exp.png"),
        make_pair(heatmap_cs_PuOr_discrete, "PuOr_discrete.png"),
        make_pair(heatmap_cs_PuOr_soft,     "PuOr_soft.png"),
        make_pair(heatmap_cs_PuOr_mixed,    "PuOr_mixed.png"),
        make_pair(heatmap_cs_PuOr_mixed_exp,"PuOr_mixed_exp.png"),
        make_pair(heatmap_cs_PuRd_discrete, "PuRd_discrete.png"),
        make_pair(heatmap_cs_PuRd_soft,     "PuRd_soft.png"),
        make_pair(heatmap_cs_PuRd_mixed,    "PuRd_mixed.png"),
        make_pair(heatmap_cs_PuRd_mixed_exp,"PuRd_mixed_exp.png"),
        make_pair(heatmap_cs_Purples_discrete, "Purples_discrete.png"),
        make_pair(heatmap_cs_Purples_soft,     "Purples_soft.png"),
        make_pair(heatmap_cs_Purples_mixed,    "Purples_mixed.png"),
        make_pair(heatmap_cs_Purples_mixed_exp,"Purples_mixed_exp.png"),
        make_pair(heatmap_cs_RdBu_discrete, "RdBu_discrete.png"),
        make_pair(heatmap_cs_RdBu_soft,     "RdBu_soft.png"),
        make_pair(heatmap_cs_RdBu_mixed,    "RdBu_mixed.png"),
        make_pair(heatmap_cs_RdBu_mixed_exp,"RdBu_mixed_exp.png"),
        make_pair(heatmap_cs_RdGy_discrete, "RdGy_discrete.png"),
        make_pair(heatmap_cs_RdGy_soft,     "RdGy_soft.png"),
        make_pair(heatmap_cs_RdGy_mixed,    "RdGy_mixed.png"),
        make_pair(heatmap_cs_RdGy_mixed_exp,"RdGy_mixed_exp.png"),
        make_pair(heatmap_cs_RdPu_discrete, "RdPu_discrete.png"),
        make_pair(heatmap_cs_RdPu_soft,     "RdPu_soft.png"),
        make_pair(heatmap_cs_RdPu_mixed,    "RdPu_mixed.png"),
        make_pair(heatmap_cs_RdPu_mixed_exp,"RdPu_mixed_exp.png"),
        make_pair(heatmap_cs_RdYlBu_discrete, "RdYlBu_discrete.png"),
        make_pair(heatmap_cs_RdYlBu_soft,     "RdYlBu_soft.png"),
        make_pair(heatmap_cs_RdYlBu_mixed,    "RdYlBu_mixed.png"),
        make_pair(heatmap_cs_RdYlBu_mixed_exp,"RdYlBu_mixed_exp.png"),
        make_pair(heatmap_cs_RdYlGn_discrete, "RdYlGn_discrete.png"),
        make_pair(heatmap_cs_RdYlGn_soft,     "RdYlGn_soft.png"),
        make_pair(heatmap_cs_RdYlGn_mixed,    "RdYlGn_mixed.png"),
        make_pair(heatmap_cs_RdYlGn_mixed_exp,"RdYlGn_mixed_exp.png"),
        make_pair(heatmap_cs_Reds_discrete, "Reds_discrete.png"),
        make_pair(heatmap_cs_Reds_soft,     "Reds_soft.png"),
        make_pair(heatmap_cs_Reds_mixed,    "Reds_mixed.png"),
        make_pair(heatmap_cs_Reds_mixed_exp,"Reds_mixed_exp.png"),
        make_pair(heatmap_cs_Spectral_discrete, "Spectral_discrete.png"),
        make_pair(heatmap_cs_Spectral_soft,     "Spectral_soft.png"),
        make_pair(heatmap_cs_Spectral_mixed,    "Spectral_mixed.png"),
        make_pair(heatmap_cs_Spectral_mixed_exp,"Spectral_mixed_exp.png"),
        make_pair(heatmap_cs_YlGnBu_discrete, "YlGnBu_discrete.png"),
        make_pair(heatmap_cs_YlGnBu_soft,     "YlGnBu_soft.png"),
        make_pair(heatmap_cs_YlGnBu_mixed,    "YlGnBu_mixed.png"),
        make_pair(heatmap_cs_YlGnBu_mixed_exp,"YlGnBu_mixed_exp.png"),
        make_pair(heatmap_cs_YlGn_discrete, "YlGn_discrete.png"),
        make_pair(heatmap_cs_YlGn_soft,     "YlGn_soft.png"),
        make_pair(heatmap_cs_YlGn_mixed,    "YlGn_mixed.png"),
        make_pair(heatmap_cs_YlGn_mixed_exp,"YlGn_mixed_exp.png"),
        make_pair(heatmap_cs_YlOrBr_discrete, "YlOrBr_discrete.png"),
        make_pair(heatmap_cs_YlOrBr_soft,     "YlOrBr_soft.png"),
        make_pair(heatmap_cs_YlOrBr_mixed,    "YlOrBr_mixed.png"),
        make_pair(heatmap_cs_YlOrBr_mixed_exp,"YlOrBr_mixed_exp.png"),
        make_pair(heatmap_cs_YlOrRd_discrete, "YlOrRd_discrete.png"),
        make_pair(heatmap_cs_YlOrRd_soft,     "YlOrRd_soft.png"),
        make_pair(heatmap_cs_YlOrRd_mixed,    "YlOrRd_mixed.png"),
        make_pair(heatmap_cs_YlOrRd_mixed_exp,"YlOrRd_mixed_exp.png"),
    };

    // First, create a heatmap which will be rendered using all colorschemes.
    heatmap_t* hm = heatmap_new(w, h);
    heatmap_stamp_t* stamp = heatmap_stamp_gen(r);

    // This creates two normal random distributions...
    random_device rd;
    mt19937 prng(rd());
    normal_distribution<float> x_distr(0.5f*w, 0.5f/3.0f*w), y_distr(0.5f*h, 0.25f*h);

    for(size_t i = 0 ; i < npoints ; ++i) {
        heatmap_add_point_with_stamp(hm, x_distr(prng), y_distr(prng), stamp);
    }

    // Next, add a "picture" of the colorscheme to the left.
    for(unsigned y = 0 ; y < h ; ++y) {
        for(unsigned x = 0 ; x < min(20UL, w) ; ++x) {
            hm->buf[y*w + x] = y*hm->max/(h-1);
        }
    }

    // Finally, render it for every single colorscheme
    vector<unsigned char> image(w*h*4);
    for(auto p : schemes) {
        cout << "rendering " << p.second << endl;
        heatmap_render_to(hm, p.first, &image[0]);

        if(unsigned error = lodepng::encode(p.second, image, w, h)) {
            cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << endl;
        }
    }

    heatmap_free(hm);
    return 0;
}
