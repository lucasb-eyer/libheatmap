#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <cmath>
#include <array>

#include "lodepng.h"
#include "heatmap.h"
#include "colorschemes/spectral_variations.h"
#include "colorschemes/gray.h"

using namespace std;

int main(int argc, char* argv[])
{
    if(argc < 3 || 4 < argc ) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " WIDTH HEIGHT [RADIUS]" << std::endl;

        return 1;
    }

    const size_t w = atoi(argv[1]), h = atoi(argv[2]), r = (argc == 4 ? atoi(argv[3]) : min(w,h)/20);
    const size_t npoints = 1000;

    pair<const heatmap_colorscheme_t*, const char*> schemes[] = {
        make_pair(heatmap_cs_spectral_discrete, "spectral_discrete.png"),
        make_pair(heatmap_cs_spectral_soft, "spectral_soft.png"),
        make_pair(heatmap_cs_spectral_mixed, "spectral_mixed.png"),
        make_pair(heatmap_cs_spectral_mixed_exp, "spectral_mixed_exp.png"),
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
