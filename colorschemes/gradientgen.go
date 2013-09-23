package main

import (
    "fmt"
    "github.com/lucasb-eyer/go-colorful"
    "image"
    "image/draw"
    "image/png"
    "math"
    "os"
    "strings"
)

const LICENSE string =
`/* heatmap - High performance heatmap creation in C.
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

`

const HEADER_TEMPLATE string =
`#ifndef _HEATMAP_COLORSCHEMES_%v_H
#define _HEATMAP_COLORSCHEMES_%v_H

#ifdef __cplusplus
extern "C" {
#endif

/* This one has only N discrete colors. */
extern const heatmap_colorscheme_t* heatmap_cs_%v_discrete;
/* This is a very soft gradient along abovementioned discrete colors. */
extern const heatmap_colorscheme_t* heatmap_cs_%v_soft;
/* This is a mix of the above two. Makes for a pretty result in many cases. */
extern const heatmap_colorscheme_t* heatmap_cs_%v_mixed;
/* An exponential version of the default mix of the above two. */
/* Use this if your maximum is very "spiked". */
extern const heatmap_colorscheme_t* heatmap_cs_%v_mixed_exp;

#ifdef __cplusplus
}
#endif

#endif /* _HEATMAP_COLORSCHEMES_%v_H */
`

// This table contains the "keypoints" of the colorgradient you want to generate.
// The position of each keypoint has to live in the range [0,1]
type GradientTable []struct {
    Col colorful.Color
    Pos float64
}

// This is the meat of the gradient computation. It returns a HCL-blend between
// the two colors around `t`.
// Note: It relies heavily on the fact that the gradient keypoints are sorted.
func (self GradientTable) GetInterpolatedColorFor(t float64) colorful.Color {
    for i := 0 ; i < len(self) - 1 ; i++ {
        c1 := self[i]
        c2 := self[i+1]
        if c1.Pos <= t && t <= c2.Pos {
            // We are in between c1 and c2. Go blend them!
            t := (t - c1.Pos)/(c2.Pos - c1.Pos)
            return c1.Col.BlendHcl(c2.Col, t).Clamped()
        }
    }

    // Nothing found? Means we're at (or past) the last gradient keypoint.
    return self[len(self)-1].Col
}

// This returns the color of the closest gradient keypoint.
// Note: This too relies on the fact that the gradient keypoints are sorted.
func (self GradientTable) GetColorFor(t float64) colorful.Color {
    for i := 0 ; i < len(self) - 1 ; i++ {
        if t < (self[i].Pos + self[i+1].Pos)*0.5 {
            return self[i].Col
        }
    }

    return self[len(self)-1].Col
}

func savepng(img image.Image, fname string) {
    toimg, err := os.Create(fname)
    if err != nil {
        fmt.Printf("Error: %v", err)
        return
    }
    defer toimg.Close()

    png.Encode(toimg, img)
}

// This is a very nice thing Golang forces you to do!
// It is necessary so that we can write out the literal of the colortable below.
func MustParseHex(s string) colorful.Color {
    c, err := colorful.Hex(s)
    if err != nil {
        panic("MustParseHex: " + err.Error())
    }
    return c
}

func main() {
    // The "keypoints" of the gradient.
    keypoints := GradientTable{
        {MustParseHex("#5e4fa2"), 0.0},
        {MustParseHex("#3288bd"), 0.1},
        {MustParseHex("#66c2a5"), 0.2},
        {MustParseHex("#abdda4"), 0.3},
        {MustParseHex("#e6f598"), 0.4},
        // {MustParseHex("#ffffbf"), 0.5},
        {MustParseHex("#fee090"), 0.6},
        {MustParseHex("#fdae61"), 0.7},
        {MustParseHex("#f46d43"), 0.8},
        {MustParseHex("#d53e4f"), 0.9},
        {MustParseHex("#9e0142"), 1.0},
    }

    name := "NAME"
    if len(os.Args) == 2 {
        name = os.Args[1]
    }

    c_file, err := os.Create(name + ".c")
    if err != nil { panic(err) }
    defer func() {
        if err := c_file.Close(); err != nil {
            panic(err)
        }
    }()

    h_file, err := os.Create(name + ".h")
    if err != nil { panic(err) }
    defer func() {
        if err := h_file.Close(); err != nil {
            panic(err)
        }
    }()

    // Might as well already create the header, we know all we need by now.
    // Since golang is lacking positional printf flags, I don't know of a
    // better way to do this, except for the template library, which as far
    // as I can tell would be a kludge here too.
    fmt.Fprintf(h_file, LICENSE + HEADER_TEMPLATE, name, name, name, name, name, name, name)

    h := 1024
    w := 40
    img_discrete  := image.NewRGBA(image.Rect(0,0,w,len(keypoints)))
    img_soft      := image.NewRGBA(image.Rect(0,0,w,h))
    img_mixed     := image.NewRGBA(image.Rect(0,0,w,h))
    img_mixed_exp := image.NewRGBA(image.Rect(0,0,w,h))

    fmt.Fprint(c_file, LICENSE)
    fmt.Fprintln(c_file, "#ifdef __cplusplus")
    fmt.Fprintln(c_file, "extern \"C\" {")
    fmt.Fprintln(c_file, "#endif\n")
    fmt.Fprintln(c_file, `#include "heatmap.h"`)
    fmt.Fprintf(c_file, `#include "colorschemes/%v.h"`+"\n\n", name)

    // discrete
    fmt.Fprintln(c_file, "static const unsigned char discrete_data[] = {")
    fmt.Fprint(c_file, "    0, 0, 0, 0")
    for y, kp := range keypoints {
        r, g, b := kp.Col.RGB255()
        fmt.Fprintf(c_file, ", %v, %v, %v, 255", r, g, b)
        draw.Draw(img_discrete, image.Rect(0, y, w, y+1), &image.Uniform{kp.Col}, image.ZP, draw.Src)
    }
    fmt.Fprintln(c_file, "\n};")
    fmt.Fprintln(c_file, "static const heatmap_colorscheme_t discrete = { discrete_data, sizeof(discrete_data)/sizeof(discrete_data[0]/4) };")
    fmt.Fprintln(c_file, "const heatmap_colorscheme_t* heatmap_cs_" + name + "_discrete = &discrete;\n")
    savepng(img_discrete, name + "_discrete.png")

    // soft
    fmt.Fprintln(c_file, "static const unsigned char soft_data[] = {")
    fmt.Fprint(c_file, "    0, 0, 0, 0")
    for y := 0 ; y < h ; y++ {
        t := float64(y)/float64(h)
        a := uint8(math.Min(t*30.0, 1.0)*255.0)
        c := keypoints.GetInterpolatedColorFor(t)
        r, g, b := c.RGB255()
        fmt.Fprintf(c_file, ", %v, %v, %v, %v", r, g, b, a)
        draw.Draw(img_soft, image.Rect(0, y, w, y+1), &image.Uniform{c}, image.ZP, draw.Src)
    }
    fmt.Fprintln(c_file, "\n};")
    fmt.Fprintln(c_file, "static const heatmap_colorscheme_t soft = { soft_data, sizeof(soft_data)/sizeof(soft_data[0]/4) };")
    fmt.Fprintln(c_file, "const heatmap_colorscheme_t* heatmap_cs_" + name + "_soft = &soft;\n")
    savepng(img_soft, name + "_soft.png")

    // mixed
    fmt.Fprintln(c_file, "static const unsigned char mixed_data[] = {")
    fmt.Fprint(c_file, "    0, 0, 0, 0")
    for y := 0 ; y < h ; y++ {
        t := float64(y)/float64(h)
        a := uint8(math.Min(t*30.0, 1.0)*255.0)
        // Here, we actually want to create a gradient overlaid with a stairs-like gradient.
        // Thus c1 is the soft gradient color and c2 the closest keypoint's color.
        c1 := keypoints.GetInterpolatedColorFor(t)
        c2 := keypoints.GetColorFor(t)
        c := c1.BlendRgb(c2, 0.2)
        r, g, b := c.RGB255()
        fmt.Fprintf(c_file, ", %v, %v, %v, %v", r, g, b, a)
        draw.Draw(img_mixed, image.Rect(0, y, w, y+1), &image.Uniform{c}, image.ZP, draw.Src)
    }
    fmt.Fprintln(c_file, "\n};")
    fmt.Fprintln(c_file, "static const heatmap_colorscheme_t mixed = { mixed_data, sizeof(mixed_data)/sizeof(mixed_data[0]/4) };")
    fmt.Fprintln(c_file, "const heatmap_colorscheme_t* heatmap_cs_" + name + "_mixed = &mixed;\n")
    savepng(img_mixed, name + "_mixed.png")

    // exp
    fmt.Fprintln(c_file, "static const unsigned char mixed_exp_data[] = {")
    fmt.Fprint(c_file, "    0, 0, 0, 0")
    for y := 0 ; y < h ; y++ {
        t := float64(y)/float64(h)
        a := uint8(math.Min(t*100.0, 1.0)*255.0) // TODO: Change a
        t = 1.0 - t;
        t = 1.0 - t*t * t*t * t*t * t*t * t*t
        c1 := keypoints.GetInterpolatedColorFor(t)
        c2 := keypoints.GetColorFor(t)
        c := c1.BlendRgb(c2, 0.2)
        r, g, b := c.RGB255()
        fmt.Fprintf(c_file, ", %v, %v, %v, %v", r, g, b, a)
        draw.Draw(img_mixed_exp, image.Rect(0, y, w, y+1), &image.Uniform{c}, image.ZP, draw.Src)
    }
    fmt.Fprintln(c_file, "\n};")
    fmt.Fprintln(c_file, "static const heatmap_colorscheme_t mixed_exp = { mixed_exp_data, sizeof(mixed_exp_data)/sizeof(mixed_exp_data[0]/4) };")
    fmt.Fprintln(c_file, "const heatmap_colorscheme_t* heatmap_cs_" + name + "_mixed_exp = &mixed_exp;\n")
    savepng(img_mixed_exp, name + "_mixed_exp.png")

    fmt.Fprintln(c_file, "#ifdef __cplusplus")
    fmt.Fprintln(c_file, "}")
    fmt.Fprintln(c_file, "#endif")
}
