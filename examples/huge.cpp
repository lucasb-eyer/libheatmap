#include <random>
#include <vector>
#include <iostream>

#include "lodepng.h"
#include "heatmap.h"

int main()
{
    std::cout << "[0/3] Initializing." << std::endl;

    static const size_t w = 16384, h = 16384, npoints = 1000;
    heatmap_t* hm = heatmap_new(w, h);

    // We definitely need a larger stamp for this huge-ass resolution!
    // A radius of 128 means we'll get a (2*128+1)²=257² pixel stamp.
    heatmap_stamp_t* stamp = heatmap_stamp_gen(128);

    // This creates two normal random distributions which will give us random points.
    std::random_device rd;
    std::mt19937 prng(rd());
    std::normal_distribution<float> x_distr(0.5f*w, 0.5f/3.0f*w), y_distr(0.5f*h, 0.25f*h);

    for(unsigned i = 0 ; i < npoints ; ++i) {
        // Notice the special function to specify the stamp.
        heatmap_add_point_with_stamp(hm, x_distr(prng), y_distr(prng), stamp);
    }

    // We're done with adding points, we don't need the stamp anymore.
    heatmap_stamp_free(stamp);

    std::cout << "[1/3] All points added to the heatmap." << std::endl;

    // The rest stays the same as in the `simple` example.

    std::vector<unsigned char> image(w*h*4);
    heatmap_render_default_to(hm, &image[0]);

    heatmap_free(hm);

    std::cout << "[2/3] Heatmap done rendering. (Saving to PNG will take some time...)" << std::endl;

    if(unsigned error = lodepng::encode("hires_heatmap.png", image, w, h)) {
        std::cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        return 1;
    }

    std::cout << "[3/3] All done, T. Hanks for your patience." << std::endl;
    return 0;
}
