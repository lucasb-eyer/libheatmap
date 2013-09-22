#include <iostream>

#include "lodepng.h"
#include "heatmap.h"

int main(int argc, char* argv[])
{
    if(argc < 3 || 5 < argc ) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " WIDTH HEIGHT [STAMP_RADIUS] < points.txt > heatmap.png" << std::endl;
        std::cout << std::endl;
        std::cout << "  points.txt should contain a list of space-separated pairs of x and y" << std::endl;
        std::cout << "  coordinates (as unsigned integers) of points to put onto the heatmap, e.g.:" << std::endl;
        std::cout << "    5 10 1 13 125 10" << std::endl;
        std::cout << "  will add the points (5, 10), (1, 13), and (125, 10) onto the map." << std::endl;
        std::cout << std::endl;
        std::cout << "  The default STAMP_RADIUS is a twentieth of the smallest heatmap dimension." << std::endl;
        std::cout << "  For instance, for a 512x1024 heatmap, the default stamp_radius is 25," << std::endl;
        std::cout << "  resulting in a stamp of 51x51 pixels." << std::endl;

        return 1;
    }

    const size_t w = atoi(argv[1]), h = atoi(argv[2]);
    heatmap_t* hm = heatmap_new(w, h);

    const size_t r = argc == 4 ? atoi(argv[3]) : std::min(w, h)/10;
    heatmap_stamp_t* stamp = heatmap_stamp_gen(r);

    unsigned int x, y;
    while(std::cin >> x >> y) {
        if(x < w && y < h) {
            heatmap_add_point_with_stamp(hm, x, y, stamp);
        } else {
            std::cerr << "Warning: Skipping out-of-bound input coordinate: (" << x << "," << y << ")." << std::endl;
        }
    }
    heatmap_stamp_free(stamp);

    std::vector<unsigned char> image(w*h*4);
    heatmap_render_default_to(hm, &image[0]);
    heatmap_free(hm);

    std::vector<unsigned char> png;
    if(unsigned error = lodepng::encode(png, image, w, h)) {
        std::cerr << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        return 1;
    }

    std::cout.write((char*)&png[0], png.size());

    return 0;
}
