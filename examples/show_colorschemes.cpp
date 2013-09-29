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
        make_pair(heatmap_cs_Blues_mixed, "Blues_mixed.png"),
        make_pair(heatmap_cs_BrBG_mixed, "BrBG_mixed.png"),
        make_pair(heatmap_cs_BuGn_mixed, "BuGn_mixed.png"),
        make_pair(heatmap_cs_BuPu_mixed, "BuPu_mixed.png"),
        make_pair(heatmap_cs_GnBu_mixed, "GnBu_mixed.png"),
        make_pair(heatmap_cs_Greens_mixed, "Greens_mixed.png"),
        make_pair(heatmap_cs_Greys_mixed, "Greys_mixed.png"),
        make_pair(heatmap_cs_Oranges_mixed, "Oranges_mixed.png"),
        make_pair(heatmap_cs_OrRd_mixed, "OrRd_mixed.png"),
        make_pair(heatmap_cs_PiYG_mixed, "PiYG_mixed.png"),
        make_pair(heatmap_cs_PRGn_mixed, "PRGn_mixed.png"),
        make_pair(heatmap_cs_PuBuGn_mixed, "PuBuGn_mixed.png"),
        make_pair(heatmap_cs_PuBu_mixed, "PuBu_mixed.png"),
        make_pair(heatmap_cs_PuOr_mixed, "PuOr_mixed.png"),
        make_pair(heatmap_cs_PuRd_mixed, "PuRd_mixed.png"),
        make_pair(heatmap_cs_Purples_mixed, "Purples_mixed.png"),
        make_pair(heatmap_cs_RdBu_mixed, "RdBu_mixed.png"),
        make_pair(heatmap_cs_RdGy_mixed, "RdGy_mixed.png"),
        make_pair(heatmap_cs_RdPu_mixed, "RdPu_mixed.png"),
        make_pair(heatmap_cs_RdYlBu_mixed, "RdYlBu_mixed.png"),
        make_pair(heatmap_cs_RdYlGn_mixed, "RdYlGn_mixed.png"),
        make_pair(heatmap_cs_Reds_mixed, "Reds_mixed.png"),
        make_pair(heatmap_cs_Spectral_mixed, "Spectral_mixed.png"),
        make_pair(heatmap_cs_YlGnBu_mixed, "YlGnBu_mixed.png"),
        make_pair(heatmap_cs_YlGn_mixed, "YlGn_mixed.png"),
        make_pair(heatmap_cs_YlOrBr_mixed, "YlOrBr_mixed.png"),
        make_pair(heatmap_cs_YlOrRd_mixed, "YlOrRd_mixed.png"),
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
        for(unsigned x = 0 ; x < min(20UL, w/5) ; ++x) {
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
