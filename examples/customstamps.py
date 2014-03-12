#!/usr/bin/env python

# heatmap - High performance heatmap creation in C.
#
# The MIT License (MIT)
#
# Copyright (c) 2013 Lucas Beyer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

from os.path import join as pjoin, dirname
from ctypes import CDLL, CFUNCTYPE, c_float, c_ulong, c_ubyte
import Image

# The stamp radius. The stamp will be a 2r+1 x 2r+1 square.
r = 15

# Load the heatmap library using ctypes
libhm = CDLL(pjoin(dirname(__file__), '..', 'libheatmap.so'))

# Create the default (round) stamp of given radius.
s_def = libhm.heatmap_stamp_gen(c_ulong(r))

# Create a custom stamp of given radius using a callback to set the stamp's content.
# The callback will be called for every pixel of the stamp, and should return the
# stamp's value at given distance to the stamp center.
# This is a convenient method to create rotationally-symmetric stamps.
HM_CB_FUNC = CFUNCTYPE(c_float, c_float)
s_fat = libhm.heatmap_stamp_gen_nonlinear(c_ulong(r), HM_CB_FUNC(lambda d: d**4))
s_pty = libhm.heatmap_stamp_gen_nonlinear(c_ulong(r), HM_CB_FUNC(lambda d: d**0.125))

# Create a custom stamp from a raw data array. The data needs to be
# laid out linearly in row-major (i.e. C) order. That means that the values
# for the pixels are ordered like:
# (x0, y0), (x1, y0), ..., (xN, y0), (x0, y1), ..., (xN, y1), ..., (xN, yM)
#
# Here, I create a "soft rectangle" stamp of fixed 10x5 size.
sw, sh = 10, 5
stampbuf = (c_float*(sw*sh))(
    0.00, 0.16, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.16, 0.00,
    0.16, 0.33, 0.66, 0.66, 0.66, 0.66, 0.66, 0.66, 0.33, 0.16,
    0.33, 0.66, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 0.66, 0.33,
    0.16, 0.33, 0.66, 0.66, 0.66, 0.66, 0.66, 0.66, 0.33, 0.16,
    0.00, 0.16, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.16, 0.00,
)
s_rct = libhm.heatmap_stamp_load(c_ulong(sw), c_ulong(sh), stampbuf)

# Create a heatmap object large enough to hold one occurrence of each stamp.
d = 2*r+1
w, h = 3*d + 10, d
hm = libhm.heatmap_new(w, h)

# Add one point with each stamp next to each other; this way we can
# see what the stamps look like.
libhm.heatmap_add_point_with_stamp(hm, c_ulong(      r), c_ulong(r), s_def)
libhm.heatmap_add_point_with_stamp(hm, c_ulong(  d + r), c_ulong(r), s_fat)
libhm.heatmap_add_point_with_stamp(hm, c_ulong(2*d + r), c_ulong(r), s_pty)
libhm.heatmap_add_point_with_stamp(hm, c_ulong(3*d + 5), c_ulong(r), s_rct)

# As soon as we're done drawing, we can free the stamps.
# (Of course, we might as well do that later.)
libhm.heatmap_stamp_free(s_def)
libhm.heatmap_stamp_free(s_fat)
libhm.heatmap_stamp_free(s_pty)
libhm.heatmap_stamp_free(s_rct)

# This creates an image out of the heatmap.
# `rawimg` now contains the image data in 32-bit RGBA.
rawimg = (c_ubyte*(w*h*4))()
libhm.heatmap_render_default_to(hm, rawimg)

# Now that we've got a finished heatmap picture, we don't need the map anymore.
libhm.heatmap_free(hm)

# Use the PIL (for example) to make a png file out of that.
img = Image.frombuffer('RGBA', (w, h), rawimg, 'raw', 'RGBA', 0, 1)
img.save('stamps.png')
