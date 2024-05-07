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

//callback function for curl
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
//Coordinate for screen is (0,0) at top left corner, x+ is horizontal right, y+ is vertical down


// Make sure we can exit gracefully when Ctrl-C is pressed.
volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

using ImageVector = std::vector<Magick::Image>;
const char* filenames[1] = {"../images/sakura.gif"};
const char* time_font_filename = "../fonts/5x8.bdf";
rgb_matrix::Font time_font;

// Given the filename, load the image and scale to the size of the
// matrix.
// // If this is an animated image, the resutlting vector will contain multiple.
static ImageVector LoadImageAndScaleImage(const char *filename, int target_width, int target_height) {
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


// Copy an image to a Canvas. Note, the RGBMatrix is implementing the Canvas
// interface as well as the FrameCanvas we use in the double-buffering of the
// animted image.
void CopyImageToCanvas(const Magick::Image &image, rgb_matrix::Canvas *canvas) {
  const int offset_x = 0, offset_y = 0;  // If you want to move the image.
  // Copy all the pixels to the canvas.
  for (size_t y = 0; y < image.rows(); ++y) {
    for (size_t x = 0; x < image.columns(); ++x) {
      const Magick::ColorRGB &c = image.pixelColor(x, y);
      if (MagickCore::ScaleQuantumToChar(c.quantumAlpha()) > 0) {
        canvas->SetPixel(x + offset_x, y + offset_y,
          static_cast<unsigned char>(c.red() * 255),
          static_cast<unsigned char>(c.green() * 255),
          static_cast<unsigned char>(c.blue() * 255));
      }
    }
  }
}

// An animated image has to constantly swap to the next frame.
// We're using double-buffering and fill an offscreen buffer first, then show.
/**
void ShowAnimatedImage(const ImageVector &images, rgb_matrix::RGBMatrix *matrix) {
  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
  while (!interrupt_received) {
    for (const auto &image : images) {
      if (interrupt_received) break;
      CopyImageToCanvas(image, offscreen_canvas);
      offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
      usleep(image.animationDelay() * 10000);  // 1/100s converted to usec
    }
  }
}
*/
void ShowAnimatedImagev2(const ImageVector &images, rgb_matrix::Canvas * canvas) {
  static int frame_num = 0;
  CopyImageToCanvas(images[frame_num], canvas);
  frame_num += 1;
  frame_num = frame_num >= (int)images.size() ? 0 : frame_num;
}

int usage(const char *progname) {
  fprintf(stderr, "Usage: %s [led-matrix-options] <image-filename>\n",
          progname);
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

//time in yyyy-mm-dd hh:mm:ss
std::vector<int> get_curr_time(){
    std::vector<int> curr_time;
    time_t now = time(0);
    tm *ltm = localtime(&now);
    curr_time.push_back(1900 + ltm->tm_year);
    curr_time.push_back(1 + ltm->tm_mon);
    curr_time.push_back(ltm->tm_mday);
    curr_time.push_back(ltm->tm_hour);
    curr_time.push_back(ltm->tm_min);
    curr_time.push_back(ltm->tm_sec);
    return curr_time;
}

void setup(){
  Magick::InitializeMagick(nullptr);
  if (!time_font.LoadFont(time_font_filename)) {
    std::cout<< "Couldn't load font "<<time_font_filename<<'\n';
  }
}

void draw_scrolling_text_on_canvas(rgb_matrix::Canvas * canvas, std::string line ,int x_Left, int x_Right , double speed){

}

void draw_weather_on_canvas(rgb_matrix::Canvas * canvas){
  static int time = 0;
  static std::string readBuffer;
  static nlohmann::json weather_json_data;
  static std::string output_line;
  if(time%1000==0){
    time = 0;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string url = "https://opendata.cwa.gov.tw/api/v1/rest/datastore/F-C0032-001";
    std::string param1 = "Authorization=your authorization code here";
    std::string param2 = "format=JSON";
    std::string param3 = "locationName=%E8%87%BA%E5%8C%97%E5%B8%82"; //unicode for taipei city
    std::string total_url = url + "?" + param1 + "&" + param2 + "&" + param3;
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, total_url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if(res != CURLE_OK){std::cout<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;}
      else{
        //std::cout<<readBuffer<<std::endl;
        weather_json_data = nlohmann::json::parse(readBuffer) ;
        std::string location = weather_json_data["records"]["location"][0]["locationName"].get<std::string>();
        std::string max_temp = weather_json_data["records"]["location"][0]["weatherElement"][4]["time"][0]["parameter"]["parameterName"].get<std::string>();
        std::string min_temp = weather_json_data["records"]["location"][0]["weatherElement"][2]["time"][0]["parameter"]["parameterName"].get<std::string>();      
        std::string precipitation_percentage = weather_json_data["records"]["location"][0]["weatherElement"][1]["time"][0]["parameter"]["parameterName"].get<std::string>();
        //std::string pp = weather_json_data["records"]["location"][0]["weatherElement"][0]["time"][0]["parameter"]["parameterName"].get<std::string>();        
        //std::cout<<pp<<std::endl;
        output_line = "Taipei City " + max_temp + "C to " + min_temp + "C, Rain " + precipitation_percentage + "%";
      }
    }    
  }

  static double precise_x = canvas->width() + 5, precise_y = 20.0;
  static int letter_spacing = 0;
  static double speed = 2.0;

  int x = (int)round(precise_x), y = (int)round(precise_y);
  rgb_matrix::Color color(255,155,155);

  int length = rgb_matrix::DrawText(canvas, time_font , x, y + time_font.baseline(), color, NULL, output_line.c_str(), 0);
  precise_x -= speed;
  if(x + length < 0){
    precise_x = canvas->width() + 5;
  }
  time++;

}

void draw_time_on_canvas(rgb_matrix::Canvas * canvas){

  rgb_matrix::Color color(255,255,255);

  std::vector<int> curr_time = get_curr_time();
  std::string line1 = std::to_string(curr_time[0]) + "/" + std::to_string(curr_time[1]) + "/" + std::to_string(curr_time[2]);
  std::string line2 = std::to_string(curr_time[3]) + ":" + std::to_string(curr_time[4]) + ":" + std::to_string(curr_time[5]);
  const char* linechar1 = line1.c_str();
  const char* linechar2 = line2.c_str();

  rgb_matrix::DrawText(canvas, time_font , 24, 0 + time_font.baseline(), color, NULL, linechar1, 0);
  rgb_matrix::DrawText(canvas, time_font , 24, 10 + time_font.baseline(), color, NULL, linechar2 , 0);
}

void draw_notify_on_canvas(rgb_matrix::Canvas * canvas){
  static double precise_x = canvas->width() + 5, precise_y = 20.0;
  static int letter_spacing = 0;
  static double speed = 1.0;

  int x = (int)round(precise_x), y = (int)round(precise_y);
  rgb_matrix::Color color(255,155,155);
  std::string line = "Nico Nico Ni";

  int length = rgb_matrix::DrawText(canvas, time_font , x, y + time_font.baseline(), color, NULL, line.c_str(), 0);
  precise_x -= speed;
  if(x + length < 0){
    precise_x = canvas->width() + 5;
  }
}

static void DrawOnCanvas(rgb_matrix::RGBMatrix *matrix){
  rgb_matrix::FrameCanvas * off_screen_canvas = matrix->CreateFrameCanvas();
  
  ImageVector back_img;
  back_img = LoadImageAndScaleImage(filenames[0], 20, 20);

  while(!interrupt_received){
    switch (back_img.size()) {
      case 0:   // failed to load image.
        std::cout << "failed to load image" << std::endl;
        break;

      case 1:   // Simple example: one image to show
        CopyImageToCanvas(back_img[0], matrix);
        break;

      default:  // More than one image: this is an animation.
        ShowAnimatedImagev2(back_img, off_screen_canvas);
        break;
    }  

    draw_time_on_canvas(off_screen_canvas);
    //draw_notify_on_canvas(off_screen_canvas);
    draw_weather_on_canvas(off_screen_canvas);

    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
    off_screen_canvas->Clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));;  // Until Ctrl-C is pressed
  }
  
}

int main(){
  //rgb_matrix::Canvas *canvas;
  rgb_matrix::RGBMatrix::Options defaults;
  rgb_matrix::RuntimeOptions runtime_opt;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.cols = 64;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = true;
  defaults.limit_refresh_rate_hz = 300;
  defaults.disable_hardware_pulsing = true;
  defaults.pwm_lsb_nanoseconds = 50;
  defaults.scan_mode = 0;

  runtime_opt.gpio_slowdown = 2;
    
  rgb_matrix::RGBMatrix *matrix = rgb_matrix::RGBMatrix::CreateFromOptions(defaults, runtime_opt);

  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  setup();
  DrawOnCanvas(matrix);    // Using the canvas.

  // Animation finished. Shut down the RGB matrix.
  matrix->Clear();
  //delete canvas;

  return 0;
}