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

#include <iostream>
#include <string.h> // memcmp
#include <cmath>

#include "heatmap.h"
#include "colorschemes/gray.h"

static float g_3x3_stamp_data[] = {
    0.0f, 0.5f, 0.0f,
    0.5f, 1.0f, 0.5f,
    0.0f, 0.5f, 0.0f
};

static heatmap_stamp_t g_3x3_stamp = {
    g_3x3_stamp_data, 3, 3
};

// Ugly and uncool globals! EVIL!
static unsigned g_failed_tests = 0;
static unsigned g_total_tests = 0;

// Ugly but cool macro hackery.
#define ENSURE_THAT(what, cond) ensure_that(what " (" #cond ")", cond, __FILE__, __LINE__)

static void ensure_that(const char* desc, bool condition, const char* file, int line)
{
    g_total_tests++;

    if(!condition) {
        std::cerr << file << ":" << line << ": Test failed: it is not true that " << desc << std::endl;
        g_failed_tests++;
    }
}

static bool almost_eq(float* a, float* b, size_t n)
{
    for(size_t i = 0 ; i < n ; ++i) {
        if(std::abs(a[i] - b[i]) > 1e-6)
            return false;
    }

    return true;
}

static bool heatmap_eq(heatmap_t* hm, float* expected)
{
    return 0 == memcmp(expected, hm->buf, sizeof(float)*hm->w*hm->h);
}

static bool stamp_eq(heatmap_stamp_t* s, float* expected)
{
    return 0 == memcmp(expected, s->buf, sizeof(float)*s->w*s->h);
}

static bool stamp_almost_eq(heatmap_stamp_t* s, float* expected)
{
    return almost_eq(s->buf, expected, s->w*s->h);
}

void test_add_nothing()
{
    heatmap_t* hm = heatmap_new(3, 3);

    static float expected[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    };


    ENSURE_THAT("the heatmap is full of zeros", heatmap_eq(hm, expected));
    ENSURE_THAT("the max of the heatmap is zero", hm->max == 0.0f);

    heatmap_free(hm);
}

void test_add_point_with_stamp_center()
{
    heatmap_t* hm = heatmap_new(3, 3);
    heatmap_add_point_with_stamp(hm, 1, 1, &g_3x3_stamp);

    ENSURE_THAT("the heatmap equals the stamp", heatmap_eq(hm, g_3x3_stamp.buf));
    ENSURE_THAT("the max of the heatmap is one", hm->max == 1.0f);

    heatmap_add_point_with_stamp(hm, 1, 1, &g_3x3_stamp);

    static float expected[] = {
        0.0f, 1.0f, 0.0f,
        1.0f, 2.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
    };

    ENSURE_THAT("the heatmap equals double the stamp", heatmap_eq(hm, expected));
    ENSURE_THAT("the max of the heatmap is two", hm->max == 2.0f);

    heatmap_free(hm);
}

void test_add_point_with_stamp_topleft()
{
    static float expected[] = {
        1.0f, 0.5f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    };

    heatmap_t* hm = heatmap_new(3, 3);
    heatmap_add_point_with_stamp(hm, 0, 0, &g_3x3_stamp);

    ENSURE_THAT("top-left point is correct", heatmap_eq(hm, expected));
    ENSURE_THAT("the max of the heatmap is one", hm->max == 1.0f);

    heatmap_free(hm);
}

void test_add_point_with_stamp_botright()
{
    static float expected[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f,
        0.0f, 0.5f, 1.0f,
    };

    heatmap_t* hm = heatmap_new(3, 3);
    heatmap_add_point_with_stamp(hm, 2, 2, &g_3x3_stamp);

    ENSURE_THAT("bot-right point is correct", heatmap_eq(hm, expected));
    ENSURE_THAT("the max of the heatmap is one", hm->max == 1.0f);

    heatmap_free(hm);
}

void test_add_point_with_stamp_outside()
{
    static float expected[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    };

    heatmap_t* hm = heatmap_new(3, 3);
    heatmap_add_point_with_stamp(hm, 3, 2, &g_3x3_stamp);
    heatmap_add_point_with_stamp(hm, 2, 3, &g_3x3_stamp);
    heatmap_add_point_with_stamp(hm, 3, 3, &g_3x3_stamp);

    ENSURE_THAT("no point outside the map got added", heatmap_eq(hm, expected));
    ENSURE_THAT("the max of the heatmap is zero", hm->max == 0.0f);

    heatmap_free(hm);
}

void test_stamp_gen()
{
    static float expected[] = {
        0.2928932f, 0.5f, 0.2928932f,
              0.5f, 1.0f, 0.5f,
        0.2928932f, 0.5f, 0.2928932f,
    };

    heatmap_stamp_t* s = heatmap_stamp_gen(1);

    ENSURE_THAT("the 1-radius stamp size is correct", s->w == 3 && s->h == 3);
    ENSURE_THAT("the 1-radius stamp data is correct", stamp_almost_eq(s, expected));

    heatmap_stamp_free(s);
}

void test_stamp_gen_nonlinear()
{
    static float ones[] = {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    static float halfs[] = {
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
    };

    static float zeros[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    };

    heatmap_stamp_t* s1 = heatmap_stamp_gen_nonlinear(1, [](float){return 0.5f;});
    heatmap_stamp_t* s2 = heatmap_stamp_gen_nonlinear(1, [](float){return -1.0f;});
    heatmap_stamp_t* s3 = heatmap_stamp_gen_nonlinear(1, [](float){return 10.0f;});

    ENSURE_THAT("the 1-radius nonlinear .5-stamp size is correct", s1->w == 3 && s1->h == 3);
    ENSURE_THAT("the 1-radius nonlinear -1-stamp size is correct", s2->w == 3 && s2->h == 3);
    ENSURE_THAT("the 1-radius nonlinear 10-stamp size is correct", s3->w == 3 && s3->h == 3);
    ENSURE_THAT("the 1-radius nonlinear .5-stamp data is correct", stamp_eq(s1, halfs));
    ENSURE_THAT("the 1-radius nonlinear -1-stamp data is correct", stamp_eq(s2, ones));
    ENSURE_THAT("the 1-radius nonlinear 10-stamp data is correct", stamp_eq(s3, zeros));

    heatmap_stamp_free(s1);
    heatmap_stamp_free(s2);
    heatmap_stamp_free(s3);
}

void test_render_to_nothing()
{
    static unsigned char expected[] = {
        0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
        0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
        0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
    };

    heatmap_t* hm = heatmap_new(3, 3);

    unsigned char img[3*3*4] = {1};
    heatmap_render_to(hm, heatmap_cs_b2w, img);
    ENSURE_THAT("empty rendered 3x3 heatmap is correct", 0 == memcmp(img, expected, 3*3*4));

    heatmap_free(hm);
}

void test_render_to_creation()
{
    static unsigned char expected[] = {
                0, 0, 0, 0,   127, 127, 127, 255,   0, 0, 0, 0,
        127, 127, 127, 255,   255, 255, 255, 255,   127, 127, 127, 255,
                0, 0, 0, 0,   127, 127, 127, 255,   0, 0, 0, 0,
    };

    heatmap_t* hm = heatmap_new(3, 3);
    heatmap_add_point_with_stamp(hm, 1, 1, &g_3x3_stamp);

    unsigned char* img = heatmap_render_to(hm, heatmap_cs_b2w, nullptr);

    ENSURE_THAT("simple rendered 3x3 heatmap is correct", 0 == memcmp(img, expected, 3*3*4));

    heatmap_free(hm);
    free(img);
}

void test_render_to_normalizing()
{
    static unsigned char expected[] = {
                0, 0, 0, 0,   127, 127, 127, 255,   0, 0, 0, 0,
        127, 127, 127, 255,   255, 255, 255, 255,   127, 127, 127, 255,
                0, 0, 0, 0,   127, 127, 127, 255,   0, 0, 0, 0,
    };

    heatmap_t* hm = heatmap_new(3, 3);
    heatmap_add_point_with_stamp(hm, 1, 1, &g_3x3_stamp);

    unsigned char img[3*3*4] = {1};
    heatmap_render_to(hm, heatmap_cs_b2w, img);

    ENSURE_THAT("simple rendered 3x3 heatmap is correct", 0 == memcmp(img, expected, 3*3*4));

    heatmap_add_point_with_stamp(hm, 1, 1, &g_3x3_stamp);
    heatmap_render_to(hm, heatmap_cs_b2w, img);

    ENSURE_THAT("two points on one spot don't change the outcome, due to normalization.", 0 == memcmp(img, expected, 3*3*4));

    heatmap_free(hm);
    // TODO: (Also try negative and non-one-max stamps?)
}

void test_render_to_saturating()
{
    static unsigned char expected1[] = {
                0, 0, 0, 0,   255, 255, 255, 255,   0, 0, 0, 0,
        255, 255, 255, 255,   255, 255, 255, 255,   255, 255, 255, 255,
                0, 0, 0, 0,   255, 255, 255, 255,   0, 0, 0, 0,
    };

    static unsigned char expected2[] = {
                0, 0, 0, 0,   191, 191, 191, 255,   0, 0, 0, 0,
        191, 191, 191, 255,   255, 255, 255, 255,   191, 191, 191, 255,
                0, 0, 0, 0,   191, 191, 191, 255,   0, 0, 0, 0,
    };

    heatmap_t* hm = heatmap_new(3, 3);
    heatmap_add_point_with_stamp(hm, 1, 1, &g_3x3_stamp);
    heatmap_add_point_with_stamp(hm, 1, 1, &g_3x3_stamp);
    heatmap_add_point_with_stamp(hm, 1, 1, &g_3x3_stamp);

    unsigned char img[3*3*4] = {1};
    heatmap_render_saturated_to(hm, heatmap_cs_b2w, 1.0f, img);
    ENSURE_THAT("saturated (1) rendered 3x3 heatmap is correct", 0 == memcmp(img, expected1, 3*3*4));

    heatmap_render_saturated_to(hm, heatmap_cs_b2w, 2.0f, img);
    ENSURE_THAT("saturated (2) rendered 3x3 heatmap is correct", 0 == memcmp(img, expected2, 3*3*4));

    heatmap_free(hm);
    // TODO: (Also try negative and non-one-max stamps?)
}

int main()
{
    test_add_nothing();
    test_add_point_with_stamp_center();
    test_add_point_with_stamp_topleft();
    test_add_point_with_stamp_botright();
    test_add_point_with_stamp_outside();

    test_stamp_gen();
    test_stamp_gen_nonlinear();

    test_render_to_nothing();
    test_render_to_creation();
    test_render_to_normalizing();
    test_render_to_saturating();

    if(g_failed_tests > 0) {
        std::cout << "Oh noes! " << g_failed_tests << " out of " << g_total_tests << " tests failed, shame on you!" << std::endl;
    } else {
        std::cout << "All " << g_total_tests << " tests succeeded. Go get the champagne!" << std::endl;
    }
    return static_cast<int>(g_failed_tests);
}
