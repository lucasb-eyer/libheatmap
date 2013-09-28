package main

import "flag"
import "log"
import "strconv"

// same thing as MustParseHex but for float parsing
func MustParseFloatZeroOne(s string) float64 {
    f, err := strconv.ParseFloat(s, 64)
    if err != nil {
        log.Fatalf("Invalid keypoint position: %v: %v\n", s, err.Error())
    }
    if f < 0.0 || 1.0 < f {
        log.Fatalf("Invalid keypoint position: %v: keypoints must lie within 0.0 and 1.0\n", s)
    }
    return f
}

func main() {
    pname := flag.String("name", "unnamed", "The name of the colorscheme.")
    pw := flag.Uint64("w", 40, "Width of the pictures of the colorscheme.")
    ph := flag.Uint64("h", 1024, "Height (number of levels) of the colorscheme.")
    flag.Parse()

    if flag.NArg() < 2*2 || flag.NArg() % 2 != 0 {
        flag.Usage()
        log.Fatal("Need at least two gradient keypoints!")
    }

    keypoints := GradientTable{}
    for i := 0 ; i < flag.NArg() ; i += 2 {
        keypoints = append(keypoints, GradientTableEntry{MustParseHex(flag.Arg(i)), MustParseFloatZeroOne(flag.Arg(i+1))})
    }

    CreateColorschemes(keypoints, *pname, int(*pw), int(*ph))
}
