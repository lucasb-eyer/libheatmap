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

#include <random>
#include <vector>
#include <iostream>

#include <string.h>
#include <errno.h>
#include <png.h>

#include "heatmap.h"

static unsigned int writepng(const std::string& filename, const unsigned char* data, size_t w, size_t h);

int main()
{
    static const size_t w = 256, h = 512, npoints = 1000;

    // Create the heatmap object with the given dimensions (in pixel).
    heatmap_t* hm = heatmap_new(w, h);

    // This creates two normal random distributions which will give us random points.
    std::random_device rd;
    std::mt19937 prng(rd());
    std::normal_distribution<float> x_distr(0.5f*w, 0.5f/3.0f*w), y_distr(0.5f*h, 0.25f*h);

    // Add a bunch of random points to the heatmap now.
    for(unsigned i = 0 ; i < npoints ; ++i) {
        heatmap_add_point(hm, x_distr(prng), y_distr(prng));
    }

    // This creates an image out of the heatmap.
    // `image` now contains the image data in 32-bit RGBA.
    std::vector<unsigned char> image(w*h*4);
    heatmap_render_default_to(hm, &image[0]);

    // Now that we've got a finished heatmap picture, we don't need the map anymore.
    heatmap_free(hm);

    // Finally, we use the libpng reference library to save it as an image.
    return writepng("heatmap.png", &image[0], w, h);
}

static unsigned int writepng(const std::string& filename, const unsigned char* data, size_t w, size_t h)
{
    // NULLS are user error/warning functions.
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if(!png_ptr) {
        std::cerr << "Error initializing libpng write struct." << std::endl;
        return 2;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
       std::cerr << "Error initializing libpng info struct." << std::endl;
       png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
       return 3;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "Error in setjmp!?" << std::endl;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 4;
    }

    FILE *fp = fopen(filename.c_str(), "wb");
    if(!fp) {
        std::cerr << "Error writing " << filename << ": " << strerror(errno) << std::endl;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 5;
    }
    png_init_io(png_ptr, fp);

    // Could be used for progress report of some sort.
    //png_set_write_status_fn(png_ptr, write_row_callback);

    // turn on or off filtering, and/or choose specific filters.
    // You can use either a single PNG_FILTER_VALUE_NAME or the logical OR
    // of one or more PNG_FILTER_NAME masks.
    png_set_filter(png_ptr, 0,
         PNG_FILTER_NONE  | PNG_FILTER_VALUE_NONE
       // | PNG_FILTER_SUB   | PNG_FILTER_VALUE_SUB
       // | PNG_FILTER_UP    | PNG_FILTER_VALUE_UP
       // | PNG_FILTER_AVE   | PNG_FILTER_VALUE_AVE
       // | PNG_FILTER_PAETH | PNG_FILTER_VALUE_PAETH
       // | PNG_ALL_FILTERS
    );

    // set the zlib compression level.
    // 1 = fast but not much compression, 9 = slow but much compression.
    png_set_compression_level(png_ptr, 1);

    static const int bit_depth = 8;
    static const int color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    static const int interlace_type = PNG_INTERLACE_ADAM7; // or PNG_INTERLACE_NONE
    png_set_IHDR(png_ptr, info_ptr, w, h, bit_depth, color_type, interlace_type, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_bytep* row_pointers = (png_byte**)png_malloc(png_ptr, h*sizeof(png_bytep));
    for(size_t y = 0 ; y < h ; ++y) {
        row_pointers[y] = const_cast<png_bytep>(data + y*w*4);
    }
    png_set_rows(png_ptr, info_ptr, row_pointers);

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    // Cleanup
    png_free(png_ptr, row_pointers);
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
}

