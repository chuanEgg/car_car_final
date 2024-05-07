CXX=g++
CXXFLAGS=-Wall -O3 -g -Wextra -Wno-unused-parameter

OBJECTS=test1.o
BINARIES=test1
RGB_LIB_DISTRIBUTION=.
RGB_INCDIR=include
RGB_LIBDIR=lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

# To compile image-example
MAGICK_CXXFLAGS?=$(shell GraphicsMagick++-config --cppflags --cxxflags)
MAGICK_LDFLAGS?=$(shell GraphicsMagick++-config --ldflags --libs)

all : $(BINARIES)

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

test1.o: test1.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

test1: $(OBJECTS) $(RGB_LIBRARY)
	$(CXX) $< -o $@ $(LDFLAGS) $(MAGICK_LDFLAGS)

FORCE:
.PHONY: FORCE