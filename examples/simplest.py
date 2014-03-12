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
from random import gauss
from ctypes import CDLL, c_ulong, c_ubyte
import Image

w, h, npoints = 256, 512, 1000

# Load the heatmap library using ctypes
libhm = CDLL(pjoin(dirname(__file__), '..', 'libheatmap.so'))

# Create the heatmap object with the given dimensions (in pixel).
hm = libhm.heatmap_new(w, h)

# Add a bunch of random points to the heatmap now.
for x, y in ((int(gauss(w*0.5, w/6.0)), int(gauss(h*0.5, h/6.0))) for _ in xrange(npoints)):
    libhm.heatmap_add_point(hm, c_ulong(x), c_ulong(y))

# This creates an image out of the heatmap.
# `rawimg` now contains the image data in 32-bit RGBA.
rawimg = (c_ubyte*(w*h*4))()
libhm.heatmap_render_default_to(hm, rawimg)

# Now that we've got a finished heatmap picture, we don't need the map anymore.
libhm.heatmap_free(hm)

# Use the PIL (for example) to make a png file out of that.
img = Image.frombuffer('RGBA', (w, h), rawimg, 'raw', 'RGBA', 0, 1)
img.save('heatmap.png')
