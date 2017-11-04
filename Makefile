CC?=gcc
CXX?=g++
AR?=ar

# Set the default optional flags if none are specified

# Release mode (If just dropping the lib into your project, check out -flto too.)
#
# Note1: OpenMP is (currently) not required by the lib, just for precise benchmarking.
# Note2: the -Wa,-ahl=... part only generates .s assembly so one can see generated code.
# Note3: If you want to add `-flto`, you should add the same -O to LDFLAGS as to FLAGS.
DEFAULT_FLAGS=-O3 -g -DNDEBUG -fopenmp -Wall -Wextra -Wa,-ahl=$(@:.o=.s)
DEFAULT_LDFLAGS=-fopenmp

# Debug mode
# DEFAULT_FLAGS=-fPIC -Wall -Wextra -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -I. -O0 -g -fopenmp -Wa,-ahl=$(@:.o=.s)
# DEFAULT_LDFLAGS=-fopenmp -g

# First set the flags to their defaults if not supplied externally.
CFLAGS?=$(DEFAULT_FLAGS)
CXXFLAGS?=$(DEFAULT_FLAGS)
LDFLAGS?=$(DEFAULT_LDFLAGS)

# Then add those flags we can't live without, unconditionally.
CFLAGS+=-fPIC -I. -pedantic
CXXFLAGS+=-fPIC -I. -std=c++0x
LDFLAGS+=-lm


.PHONY: all benchmarks samples clean

all: libheatmap.a libheatmap.so benchmarks examples tests
tests: tests/test
benchmarks: benchs/add_point_with_stamp benchs/weighted_unweighted benchs/rendering
examples: examples/heatmap_gen examples/heatmap_gen_weighted examples/simplest_cpp examples/simplest_c examples/huge examples/customstamps examples/customstamp_heatmaps examples/show_colorschemes

clean:
	rm -f libheatmap.a
	rm -f libheatmap.so
	rm -f benchs/add_point_with_stamp
	rm -f benchs/rendering
	rm -f examples/heatmap_gen
	rm -f examples/heatmap_gen_weighted
	rm -f examples/simplest_c
	rm -f examples/simplest_cpp
	rm -f examples/simplest_libpng_cpp
	rm -f examples/huge
	rm -f examples/customstamps
	rm -f examples/customstamp_heatmaps
	rm -f examples/show_colorschemes
	rm -f tests/test
	find . -name '*.[os]' -print0 | xargs -0 rm -f

test: tests
	tests/test

heatmap.o: heatmap.c heatmap.h
	$(CC) -c $< $(CFLAGS) -o $@

colorschemes/%.o: colorschemes/%.c colorschemes/%.h
	$(CC) -c $< $(CFLAGS) -o $@

libheatmap.a: heatmap.o $(patsubst %.c,%.o,$(wildcard colorschemes/*.c))
	$(AR) rs $@ $^

libheatmap.so: heatmap.o $(patsubst %.c,%.o,$(wildcard colorschemes/*.c))
	$(CC) $(LDFLAGS) -shared -o $@ $^

tests/test.o: tests/test.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

tests/test: tests/test.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

examples/lodepng_cpp.o: examples/lodepng.cpp examples/lodepng.h
	$(CXX) -c $< $(CXXFLAGS) -o $@

examples/lodepng_c.o: examples/lodepng.cpp examples/lodepng.h
	$(CC) -x c -c $< $(CFLAGS) -o $@

examples/heatmap_gen.o: examples/heatmap_gen.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

examples/heatmap_gen: examples/heatmap_gen.o examples/lodepng_cpp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

examples/heatmap_gen_weighted.o: examples/heatmap_gen.cpp
	$(CXX) -c $< $(CXXFLAGS) -DWEIGHTED -o $@

examples/heatmap_gen_weighted: examples/heatmap_gen_weighted.o examples/lodepng_cpp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

examples/simplest_cpp.o: examples/simplest.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

examples/simplest_cpp: examples/simplest_cpp.o examples/lodepng_cpp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

examples/simplest_c.o: examples/simplest.c
	$(CC) -c $< $(CFLAGS) -o $@

examples/simplest_c: examples/simplest_c.o examples/lodepng_c.o libheatmap.a
	$(CC) $^ $(LDFLAGS) -o $@

examples/simplest_libpng_cpp.o: examples/simplest_libpng.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

examples/simplest_libpng_cpp: examples/simplest_libpng_cpp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -lpng -o $@

examples/huge.o: examples/huge.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

examples/huge: examples/huge.o examples/lodepng_cpp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

examples/customstamps.o: examples/customstamps.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

examples/customstamps: examples/customstamps.o examples/lodepng_cpp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

examples/customstamp_heatmaps.o: examples/customstamp_heatmaps.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

examples/customstamp_heatmaps: examples/customstamp_heatmaps.o examples/lodepng_cpp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

examples/show_colorschemes.o: examples/show_colorschemes.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

examples/show_colorschemes: examples/show_colorschemes.o examples/lodepng_cpp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

benchs/add_point_with_stamp.o: benchs/add_point_with_stamp.cpp benchs/common.hpp benchs/timing.hpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

benchs/add_point_with_stamp: benchs/add_point_with_stamp.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

benchs/weighted_unweighted.o: benchs/weighted_unweighted.cpp benchs/common.hpp benchs/timing.hpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

benchs/weighted_unweighted: benchs/weighted_unweighted.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@

benchs/rendering.o: benchs/rendering.cpp benchs/common.hpp benchs/timing.hpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

benchs/rendering: benchs/rendering.o libheatmap.a
	$(CXX) $^ $(LDFLAGS) -o $@
