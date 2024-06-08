#include "draw_tools.h"

int usage(const char *progname) {
  fprintf(stderr, "Usage: %s [led-matrix-options] <image-filename>\n",
          progname);
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

void InterruptHandler(int signo) {
  interrupt_received = true;
}

ImageVector LoadImageAndScaleImage(const char *filename, int target_width, int target_height) {
  ImageVector result;

  ImageVector frames;
  try {
    readImages(&frames, filename);
  } catch (std::exception &e) {
    if (e.what())
      fprintf(stderr, "%s\n", e.what());
    return result;
  }

  if (frames.empty()) {
    fprintf(stderr, "No image found.");
    return result;
  }

  // Animated images have partial frames that need to be put together
  if (frames.size() > 1) {
    Magick::coalesceImages(&result, frames.begin(), frames.end());
  } else {
    result.push_back(frames[0]); // just a single still image.
  }

  for (Magick::Image &image : result) {
    image.scale(Magick::Geometry(target_width, target_height));
  }

  return result;
}

void CopyImageToCanvas(const Magick::Image &image, rgb_matrix::Canvas *canvas, int x_coordinate, int y_coordinate) {
  // const int offset_x = 0, offset_y = 0;  // If you want to move the image.
  // Copy all the pixels to the canvas.
  for (size_t y = 0; y < image.rows(); ++y) {
    for (size_t x = 0; x < image.columns(); ++x) {
      const Magick::ColorRGB &c = image.pixelColor(x, y);
      if (MagickCore::ScaleQuantumToChar(c.quantumAlpha()) > 0) {
        canvas->SetPixel(x + x_coordinate, y + y_coordinate,
          static_cast<unsigned char>(c.red() * 255),
          static_cast<unsigned char>(c.green() * 255),
          static_cast<unsigned char>(c.blue() * 255));
      }
    }
  }
}

void ShowAnimatedImagev2(const ImageVector &images, rgb_matrix::Canvas * canvas, int x_coordinate, int y_coordinate) {
  static int frame_num = 0;
  CopyImageToCanvas(images.at(frame_num), canvas, x_coordinate, y_coordinate);
  frame_num += 1;
  frame_num = frame_num >= (int)images.size() ? 0 : frame_num;
}




