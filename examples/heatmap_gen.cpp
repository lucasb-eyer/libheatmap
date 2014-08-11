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
#include <string>
#include <map>

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

// Too bad C++ doesn't have nice mad reflection skillz!
std::map<std::string, const heatmap_colorscheme_t*> g_schemes = {
    {"b2w", heatmap_cs_b2w},
    {"b2w_opaque", heatmap_cs_b2w_opaque},
    {"w2b", heatmap_cs_w2b},
    {"w2b_opaque", heatmap_cs_w2b_opaque},
    {"Blues_discrete", heatmap_cs_Blues_discrete},
    {"Blues_soft", heatmap_cs_Blues_soft},
    {"Blues_mixed", heatmap_cs_Blues_mixed},
    {"Blues_mixed_exp", heatmap_cs_Blues_mixed_exp},
    {"BrBG_discrete", heatmap_cs_BrBG_discrete},
    {"BrBG_soft", heatmap_cs_BrBG_soft},
    {"BrBG_mixed", heatmap_cs_BrBG_mixed},
    {"BrBG_mixed_exp", heatmap_cs_BrBG_mixed_exp},
    {"BuGn_discrete", heatmap_cs_BuGn_discrete},
    {"BuGn_soft", heatmap_cs_BuGn_soft},
    {"BuGn_mixed", heatmap_cs_BuGn_mixed},
    {"BuGn_mixed_exp", heatmap_cs_BuGn_mixed_exp},
    {"BuPu_discrete", heatmap_cs_BuPu_discrete},
    {"BuPu_soft", heatmap_cs_BuPu_soft},
    {"BuPu_mixed", heatmap_cs_BuPu_mixed},
    {"BuPu_mixed_exp", heatmap_cs_BuPu_mixed_exp},
    {"GnBu_discrete", heatmap_cs_GnBu_discrete},
    {"GnBu_soft", heatmap_cs_GnBu_soft},
    {"GnBu_mixed", heatmap_cs_GnBu_mixed},
    {"GnBu_mixed_exp", heatmap_cs_GnBu_mixed_exp},
    {"Greens_discrete", heatmap_cs_Greens_discrete},
    {"Greens_soft", heatmap_cs_Greens_soft},
    {"Greens_mixed", heatmap_cs_Greens_mixed},
    {"Greens_mixed_exp", heatmap_cs_Greens_mixed_exp},
    {"Greys_discrete", heatmap_cs_Greys_discrete},
    {"Greys_soft", heatmap_cs_Greys_soft},
    {"Greys_mixed", heatmap_cs_Greys_mixed},
    {"Greys_mixed_exp", heatmap_cs_Greys_mixed_exp},
    {"Oranges_discrete", heatmap_cs_Oranges_discrete},
    {"Oranges_soft", heatmap_cs_Oranges_soft},
    {"Oranges_mixed", heatmap_cs_Oranges_mixed},
    {"Oranges_mixed_exp", heatmap_cs_Oranges_mixed_exp},
    {"OrRd_discrete", heatmap_cs_OrRd_discrete},
    {"OrRd_soft", heatmap_cs_OrRd_soft},
    {"OrRd_mixed", heatmap_cs_OrRd_mixed},
    {"OrRd_mixed_exp", heatmap_cs_OrRd_mixed_exp},
    {"PiYG_discrete", heatmap_cs_PiYG_discrete},
    {"PiYG_soft", heatmap_cs_PiYG_soft},
    {"PiYG_mixed", heatmap_cs_PiYG_mixed},
    {"PiYG_mixed_exp", heatmap_cs_PiYG_mixed_exp},
    {"PRGn_discrete", heatmap_cs_PRGn_discrete},
    {"PRGn_soft", heatmap_cs_PRGn_soft},
    {"PRGn_mixed", heatmap_cs_PRGn_mixed},
    {"PRGn_mixed_exp", heatmap_cs_PRGn_mixed_exp},
    {"PuBuGn_discrete", heatmap_cs_PuBuGn_discrete},
    {"PuBuGn_soft", heatmap_cs_PuBuGn_soft},
    {"PuBuGn_mixed", heatmap_cs_PuBuGn_mixed},
    {"PuBuGn_mixed_exp", heatmap_cs_PuBuGn_mixed_exp},
    {"PuBu_discrete", heatmap_cs_PuBu_discrete},
    {"PuBu_soft", heatmap_cs_PuBu_soft},
    {"PuBu_mixed", heatmap_cs_PuBu_mixed},
    {"PuBu_mixed_exp", heatmap_cs_PuBu_mixed_exp},
    {"PuOr_discrete", heatmap_cs_PuOr_discrete},
    {"PuOr_soft", heatmap_cs_PuOr_soft},
    {"PuOr_mixed", heatmap_cs_PuOr_mixed},
    {"PuOr_mixed_exp", heatmap_cs_PuOr_mixed_exp},
    {"PuRd_discrete", heatmap_cs_PuRd_discrete},
    {"PuRd_soft", heatmap_cs_PuRd_soft},
    {"PuRd_mixed", heatmap_cs_PuRd_mixed},
    {"PuRd_mixed_exp", heatmap_cs_PuRd_mixed_exp},
    {"Purples_discrete", heatmap_cs_Purples_discrete},
    {"Purples_soft", heatmap_cs_Purples_soft},
    {"Purples_mixed", heatmap_cs_Purples_mixed},
    {"Purples_mixed_exp", heatmap_cs_Purples_mixed_exp},
    {"RdBu_discrete", heatmap_cs_RdBu_discrete},
    {"RdBu_soft", heatmap_cs_RdBu_soft},
    {"RdBu_mixed", heatmap_cs_RdBu_mixed},
    {"RdBu_mixed_exp", heatmap_cs_RdBu_mixed_exp},
    {"RdGy_discrete", heatmap_cs_RdGy_discrete},
    {"RdGy_soft", heatmap_cs_RdGy_soft},
    {"RdGy_mixed", heatmap_cs_RdGy_mixed},
    {"RdGy_mixed_exp", heatmap_cs_RdGy_mixed_exp},
    {"RdPu_discrete", heatmap_cs_RdPu_discrete},
    {"RdPu_soft", heatmap_cs_RdPu_soft},
    {"RdPu_mixed", heatmap_cs_RdPu_mixed},
    {"RdPu_mixed_exp", heatmap_cs_RdPu_mixed_exp},
    {"RdYlBu_discrete", heatmap_cs_RdYlBu_discrete},
    {"RdYlBu_soft", heatmap_cs_RdYlBu_soft},
    {"RdYlBu_mixed", heatmap_cs_RdYlBu_mixed},
    {"RdYlBu_mixed_exp", heatmap_cs_RdYlBu_mixed_exp},
    {"RdYlGn_discrete", heatmap_cs_RdYlGn_discrete},
    {"RdYlGn_soft", heatmap_cs_RdYlGn_soft},
    {"RdYlGn_mixed", heatmap_cs_RdYlGn_mixed},
    {"RdYlGn_mixed_exp", heatmap_cs_RdYlGn_mixed_exp},
    {"Reds_discrete", heatmap_cs_Reds_discrete},
    {"Reds_soft", heatmap_cs_Reds_soft},
    {"Reds_mixed", heatmap_cs_Reds_mixed},
    {"Reds_mixed_exp", heatmap_cs_Reds_mixed_exp},
    {"Spectral_discrete", heatmap_cs_Spectral_discrete},
    {"Spectral_soft", heatmap_cs_Spectral_soft},
    {"Spectral_mixed", heatmap_cs_Spectral_mixed},
    {"Spectral_mixed_exp", heatmap_cs_Spectral_mixed_exp},
    {"YlGnBu_discrete", heatmap_cs_YlGnBu_discrete},
    {"YlGnBu_soft", heatmap_cs_YlGnBu_soft},
    {"YlGnBu_mixed", heatmap_cs_YlGnBu_mixed},
    {"YlGnBu_mixed_exp", heatmap_cs_YlGnBu_mixed_exp},
    {"YlGn_discrete", heatmap_cs_YlGn_discrete},
    {"YlGn_soft", heatmap_cs_YlGn_soft},
    {"YlGn_mixed", heatmap_cs_YlGn_mixed},
    {"YlGn_mixed_exp", heatmap_cs_YlGn_mixed_exp},
    {"YlOrBr_discrete", heatmap_cs_YlOrBr_discrete},
    {"YlOrBr_soft", heatmap_cs_YlOrBr_soft},
    {"YlOrBr_mixed", heatmap_cs_YlOrBr_mixed},
    {"YlOrBr_mixed_exp", heatmap_cs_YlOrBr_mixed_exp},
    {"YlOrRd_discrete", heatmap_cs_YlOrRd_discrete},
    {"YlOrRd_soft", heatmap_cs_YlOrRd_soft},
    {"YlOrRd_mixed", heatmap_cs_YlOrRd_mixed},
    {"YlOrRd_mixed_exp", heatmap_cs_YlOrRd_mixed_exp},
};

int main(int argc, char* argv[])
{
    if(argc == 2 && std::string(argv[1]) == "-l") {
        for(auto& scheme : g_schemes) {
            std::cout << "  " << scheme.first << std::endl;
        }
        return 0;
    }

    if(argc < 3 || 6 < argc) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " WIDTH HEIGHT [STAMP_RADIUS [COLORSCHEME]] < points.txt > heatmap.png" << std::endl;
        std::cout << std::endl;
        std::cout << "  points.txt should contain a list of space-separated pairs of x and y" << std::endl;
        std::cout << "  coordinates (as unsigned integers) of points to put onto the heatmap, e.g.:" << std::endl;
        std::cout << "    5 10 1 13 125 10" << std::endl;
        std::cout << "  will add the points (5, 10), (1, 13), and (125, 10) onto the map." << std::endl;
        std::cout << std::endl;
        std::cout << "  The default STAMP_RADIUS is a twentieth of the smallest heatmap dimension." << std::endl;
        std::cout << "  For instance, for a 512x1024 heatmap, the default stamp_radius is 25," << std::endl;
        std::cout << "  resulting in a stamp of 51x51 pixels." << std::endl;
        std::cout << std::endl;
        std::cout << "  To get a list of available colorschemes, run" << std::endl;
        std::cout << "  " << argv[0] << " -l" << std::endl;
        std::cout << "  The default colorscheme is Spectral_mixed." << std::endl;

        return 1;
    }

    const size_t w = atoi(argv[1]), h = atoi(argv[2]);
    heatmap_t* hm = heatmap_new(w, h);

    const size_t r = argc >= 4 ? atoi(argv[3]) : std::min(w, h)/10;
    heatmap_stamp_t* stamp = heatmap_stamp_gen(r);

    if(argc >= 5 && g_schemes.find(argv[4]) == g_schemes.end()) {
        std::cerr << "Unknown colorscheme. Run " << argv[0] << " -l for a list of valid ones." << std::endl;
        return 1;
    }
    const heatmap_colorscheme_t* colorscheme = argc == 5 ? g_schemes[argv[4]] : heatmap_cs_default;

    unsigned int x, y;
    while(std::cin >> x >> y) {
        x /= 7.998427672955975; y /= 7.998427672955975;
        if(x < w && y < h) {
            heatmap_add_point_with_stamp(hm, x, y, stamp);
        } else {
            std::cerr << "Warning: Skipping out-of-bound input coordinate: (" << x << "," << y << ")." << std::endl;
        }
    }
    heatmap_stamp_free(stamp);

    std::vector<unsigned char> image(w*h*4);
    heatmap_render_to(hm, colorscheme, &image[0]);
    heatmap_free(hm);

    std::vector<unsigned char> png;
    if(unsigned error = lodepng::encode(png, image, w, h)) {
        std::cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        return 1;
    }

    std::cout.write((char*)&png[0], png.size());

    return 0;
}
