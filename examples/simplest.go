package main

import (
	"image"
	"os"
	"log"
	"image/png"
)

/*
#cgo CFLAGS: -O3
#include <stdint.h>
#include "heatmap.h"
*/
import "C"

func main() {
	imageData := make([]uint8,256*256*4)

	heatmap := C.heatmap_new(256, 256)
	C.heatmap_add_point(heatmap, 100, 100)
	C.heatmap_render_default_to(heatmap, (*C.uchar)(&imageData[0]))
	C.heatmap_free(heatmap)

	img := image.NewNRGBA(image.Rect(0, 0, 256, 256))

	img.Pix = imageData

	f, err := os.Create("image.png")
	if err != nil {
		log.Fatal(err)
	}

	if err := png.Encode(f, img); err != nil {
		f.Close()
		log.Fatal(err)
	}

	if err := f.Close(); err != nil {
		log.Fatal(err)
	}
}