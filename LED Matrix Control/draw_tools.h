#ifndef DRAW_TOOLS_H
#define DRAW_TOOLS_H

#include <iostream>
#include <stdio.h>

#include "include/led-matrix.h"
#include "include/graphics.h"

#include <math.h>
#include <signal.h>

#include <thread>
#include <string>
#include <vector>

#include <chrono>

#include <exception>
#include <Magick++.h>

#include <curl/curl.h>

#include <nlohmann/json.hpp>

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

using ImageVector = std::vector<Magick::Image>;
//volatile bool interrupt_received = false;
extern volatile bool interrupt_received;

int usage(const char *progname);

//Coordinate for screen is (0,0) at top left corner, x+ is horizontal right, y+ is vertical down


// Make sure we can exit gracefully when Ctrl-C is pressed.
void InterruptHandler(int signo);


// Given the filename, load the image and scale to the size of the
// matrix.
// // If this is an animated image, the resutlting vector will contain multiple.
ImageVector LoadImageAndScaleImage(const char *filename, int target_width, int target_height);


// Copy an image to a Canvas. Note, the RGBMatrix is implementing the Canvas
// interface as well as the FrameCanvas we use in the double-buffering of the
// animted image.
void CopyImageToCanvas(const Magick::Image &image, rgb_matrix::Canvas *canvas, int x_coordinate, int y_coordinate);

// An animated image has to constantly swap to the next frame.
// We're using double-buffering and fill an offscreen buffer first, then show.
// x and y are the coordinates on screen where the top left corner of the image will be placed.
void ShowAnimatedImagev2(const ImageVector &images, rgb_matrix::Canvas * canvas, int x_coordinate, int y_coordinate);

#endif
