#include <iostream>
#include <stdio.h>

#include "include/led-matrix.h"
#include "include/graphics.h"

#include <math.h>
#include <signal.h>
#include <thread>
#include <string>
#include <vector>
#include <exception>
#include <chrono>

#include <Magick++.h>

#include <curl/curl.h>

#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include "draw_tools.h"
#include "modules.h"
#include "database.h"
#include "weather.h"


enum page_type {
    CLOCK,
    WEATHER_36HR_FORECAST,
};

enum weather_type {
    CURRENT,
    FORECAST
};

volatile bool interrupt_received = false;
struct gpiod_chip *chip = nullptr;

const char* filenames[1] = {"../images/sakura.gif"};
const char* time_font_filename = "../fonts/5x7.bdf";
const char* database_filename = "../../SQLite Database/database.db";

rgb_matrix::Font time_font;
SHT30 *sht30;
Push_Button *button1;

Database *db;

page_type& operator++(page_type& type){
    switch(type){
        case CLOCK:
            type = WEATHER_36HR_FORECAST;
            break;
        case WEATHER_36HR_FORECAST:
            type = CLOCK;
            break;
    }
    return type;
}

page_type& operator--(page_type& type){
    switch(type){
        case CLOCK:
            type = WEATHER_36HR_FORECAST;
            break;
        case WEATHER_36HR_FORECAST:
            type = CLOCK;
            break;
    }
    return type;
}

page_type operator++(page_type& type, int num){
    ++type;
    return type;
}

page_type operator--(page_type& type, int num){
    --type;
    return type;
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

  //sht30 = new SHT30();
  db = new Database(database_filename);

}

void clean_up(){
  //sht30->~SHT30();
  db->~Database();

}

void draw_scrolling_text_on_canvas(rgb_matrix::Canvas * canvas, std::string line ,int x_Left, int x_Right , double speed){

}

void draw_weather_on_canvas(rgb_matrix::Canvas * canvas,int location_code){
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


void draw_weather_on_canvas_v2(rgb_matrix::Canvas * canvas,int location_code){
  static int time = 0;

  static std::string output_line;
  static int last_location_code = 100;
  if(time%1000==0 || last_location_code != location_code){
    last_location_code = location_code;
    time = 0;

    City city = db->get_city(location_code);
    WeatherData_days_per_12hr weather_data = get_city_weather_data(city);
    /*
    nlohmann::json weather_json_data;
    std::string readBuffer;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string url = "https://opendata.cwa.gov.tw/api/v1/rest/datastore/F-C0032-001";
    std::string param1 = "Authorization=your authorization code here";
    std::string param2 = "format=JSON";
    std::string param3 = "locationName=" + city.Chinese_name_city;
    std::cout<<param3<<std::endl;
    std::string total_url = url + "?" + param1 + "&" + param2 + "&" + param3;
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, total_url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if(res != CURLE_OK){std::cerr<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;}
      else{
        //std::cout<<readBuffer<<std::endl;
        weather_json_data = nlohmann::json::parse(readBuffer) ;
        std::string location = weather_json_data["records"]["location"][0]["locationName"].get<std::string>();
        std::string max_temp = weather_json_data["records"]["location"][0]["weatherElement"][4]["time"][0]["parameter"]["parameterName"].get<std::string>();
        std::string min_temp = weather_json_data["records"]["location"][0]["weatherElement"][2]["time"][0]["parameter"]["parameterName"].get<std::string>();      
        std::string precipitation_percentage = weather_json_data["records"]["location"][0]["weatherElement"][1]["time"][0]["parameter"]["parameterName"].get<std::string>();
        //std::string pp = weather_json_data["records"]["location"][0]["weatherElement"][0]["time"][0]["parameter"]["parameterName"].get<std::string>();        
        //std::cout<<pp<<std::endl;
        output_line = city.English_name_city + " " + max_temp + "C to " + min_temp + "C, Rain " + precipitation_percentage + "%";
      }
    } 
    */
    std::cout<<city.English_name_city<<std::endl;
    output_line = city.English_name_city + " " + weather_data.MaxT[0] + "C to " + weather_data.MinT[0] + "C, Rain " + weather_data.PoP12h[0] + "%";   
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

void draw_36hr_weather_forecast_on_canvas(rgb_matrix::Canvas * canvas,int location_code){
  static int time = 0;

  static std::string output_line1, output_line2, output_line3, output_line4;
  static int last_location_code = 100;
  if(time%1000==0 || last_location_code != location_code){
    last_location_code = location_code;
    time = 0;

    City city = db->get_city(location_code);

    WeatherData_days_per_12hr weather_data = get_city_township_weather_data(city);

    weather_data.Time[0] = weather_data.Time[0].substr(5,2) + "/" + weather_data.Time[0].substr(8,2) + " " + weather_data.Time[0].substr(11,2) + "h";
    weather_data.Time[1] = weather_data.Time[1].substr(5,2) + "/" + weather_data.Time[1].substr(8,2) + " " + weather_data.Time[1].substr(11,2) + "h";
    weather_data.Time[2] = weather_data.Time[2].substr(5,2) + "/" + weather_data.Time[2].substr(8,2) + " " + weather_data.Time[2].substr(11,2) + "h";


    output_line1 = city.English_name_township + ',' +city.English_name_city;
    output_line2 = weather_data.Time[0] + " " + weather_data.T[0] + "C " + weather_data.PoP12h[0] + "%";
    output_line3 = weather_data.Time[1] + " " + weather_data.T[1] + "C " + weather_data.PoP12h[1] + "%";
    output_line4 = weather_data.Time[2] + " " + weather_data.T[2] + "C " + weather_data.PoP12h[2] + "%";
    
    /*

    nlohmann::json weather_json_data;
    std::string readBuffer;


    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string url = "https://opendata.cwa.gov.tw/api/v1/rest/datastore/F-D0047-" + city.Weather_Forecast_Code_2_Days;
    std::string param1 = "Authorization=your authorization code here";
    std::string param2 = "format=JSON";
    std::string param3 = "locationName=" + city.Chinese_name_township;
    std::string param4 = "elementName=PoP12h,AT,T";
    std::string total_url = url + "?" + param1 + "&" + param2 + "&" + param3 + "&" + param4;
    
    std::cout<<total_url<<std::endl;

    //total_url = "https://opendata.cwa.gov.tw/api/v1/rest/datastore/F-D0047-009?Authorization=your authorization code here&format=JSON&locationName=%E7%AB%B9%E5%8C%97%E5%B8%82&elementName=PoP12h,AT,T";

    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, total_url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if(res != CURLE_OK){std::cerr<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;}
      else{
        weather_json_data = nlohmann::json::parse(readBuffer) ;

        //std::cout<<"in place \n";

        std::string PoP12_1 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][0]["elementValue"][0]["value"].get<std::string>();
        std::string PoP12_2 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][1]["elementValue"][0]["value"].get<std::string>();
        std::string PoP12_3 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][2]["elementValue"][0]["value"].get<std::string>();

        //std::cout<<"PoP12_1: "<<PoP12_1<<std::endl;

        std::string ATemp_1 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][1]["time"][0]["elementValue"][0]["value"].get<std::string>();
        std::string ATemp_2 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][1]["time"][1]["elementValue"][0]["value"].get<std::string>();
        std::string ATemp_3 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][1]["time"][2]["elementValue"][0]["value"].get<std::string>();

        //std::cout<<"ATemp_1: "<<ATemp_1<<std::endl;

        std::string Temp_1 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][0]["elementValue"][0]["value"].get<std::string>();
        std::string Temp_2 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][1]["elementValue"][0]["value"].get<std::string>();
        std::string Temp_3 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][2]["elementValue"][0]["value"].get<std::string>();

        //std::cout<<"Temp_1: "<<Temp_1<<std::endl;

        std::string Time_1 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][0]["dataTime"].get<std::string>();
        std::string Time_2 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][1]["dataTime"].get<std::string>();
        std::string Time_3 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][2]["dataTime"].get<std::string>();

        //std::cout<<"Time_1: "<<Time_1<<std::endl;

        Time_1 = Time_1.substr(5,2) + "/" + Time_1.substr(8,2) + " " + Time_1.substr(11,2) + "h";
        Time_2 = Time_2.substr(5,2) + "/" + Time_2.substr(8,2) + " " + Time_2.substr(11,2) + "h";
        Time_3 = Time_3.substr(5,2) + "/" + Time_3.substr(8,2) + " " + Time_3.substr(11,2) + "h";

        output_line1 = city.English_name_township + ',' +city.English_name_city;
        output_line2 = Time_1 + " " + Temp_1 + "C " + PoP12_1 + "%";
        output_line3 = Time_2 + " " + Temp_2 + "C " + PoP12_2 + "%";
        output_line4 = Time_3 + " " + Temp_3 + "C " + PoP12_3 + "%";

        std::cout<<output_line1<<std::endl;
        std::cout<<output_line2<<std::endl;
        std::cout<<output_line3<<std::endl;
        std::cout<<output_line4<<std::endl;
      }
    }
    */


  }
  time++;

  rgb_matrix::Color color(255,255,255);
  
  static double precise_x = canvas->width() + 5, precise_y = 0.0;
  static int letter_spacing = 0;
  static double speed = 2.0;

  int x = (int)round(precise_x), y = (int)round(precise_y);

  int length = rgb_matrix::DrawText(canvas, time_font , x, y + time_font.baseline(), color, NULL, output_line1.c_str(), 0);
  rgb_matrix::DrawText(canvas, time_font , 0, y + time_font.baseline()*2+1, color, NULL, output_line2.c_str(), 0);
  rgb_matrix::DrawText(canvas, time_font , 0, y + time_font.baseline()*3+2, color, NULL, output_line3.c_str(), 0);
  rgb_matrix::DrawText(canvas, time_font , 0, y + time_font.baseline()*4+3, color, NULL, output_line4.c_str(), 0);

  precise_x -= speed;
  if(x + length < 0){
    precise_x = canvas->width() + 5;
  }
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

void draw_number_on_canvas(rgb_matrix::Canvas * canvas, int num){
  rgb_matrix::Color color(255,155,155);
  rgb_matrix::DrawText(canvas, time_font , 0, 32, color, NULL, std::to_string(num).c_str(), 0);
}

void change_page(page_type &current_page,int num){
  if(num > 0){
    for(int i = 0 ; i < num ; i++){
      ++current_page;
    }    
  }
  else if(num < 0){
    for(int i = 0 ; i > num ; i--){
      --current_page;
    }
  }

}

static void CLOCK_Page(rgb_matrix::RGBMatrix *matrix, page_type &current_page){
  rgb_matrix::FrameCanvas * off_screen_canvas = matrix->CreateFrameCanvas();
  
  ImageVector back_img;
  back_img = LoadImageAndScaleImage(filenames[0], 20, 20);

  while(!interrupt_received && current_page == CLOCK){

    static std::pair<int,int> direction_and_location;
    static int cnt = 0;
    if(cnt%10==0){
      direction_and_location = db->get_direction_and_location();
      change_page(current_page, direction_and_location.first);
      cnt=0;
    }
    cnt++;


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
    draw_weather_on_canvas_v2(off_screen_canvas,direction_and_location.second);

    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
    off_screen_canvas->Clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));;  // Until Ctrl-C is pressed

  }
  
}

/*
static void WEATHER_Page(rgb_matrix::RGBMatrix *matrix, page_type &current_page){

  

  rgb_matrix::FrameCanvas * off_screen_canvas = matrix->CreateFrameCanvas();
  ImageVector back_img;
  back_img = LoadImageAndScaleImage(filenames[0], 15, 15);

  while(!interrupt_received && current_page == WEATHER){

    static std::pair<int,int> direction_and_location;
    static int cnt = 0;
    if(cnt%10==0){
      direction_and_location = db->get_direction_and_location();
      change_page(current_page,direction_and_location.first);
      cnt=0;
    }
    cnt++;


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
    draw_weather_on_canvas_v2(off_screen_canvas,direction_and_location.second);
    //draw_number_on_canvas(off_screen_canvas, direction_and_location.second);
    

    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
    off_screen_canvas->Clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));;  // Until Ctrl-C is pressed
  }
  
}
*/


static void WEATHER_36HR_FORECAST_Page(rgb_matrix::RGBMatrix *matrix, page_type &current_page){
  rgb_matrix::FrameCanvas * off_screen_canvas = matrix->CreateFrameCanvas();

  while(!interrupt_received && current_page == WEATHER_36HR_FORECAST){

    static std::pair<int,int> direction_and_location;
    static int cnt = 0;
    if(cnt%10==0){
      direction_and_location = db->get_direction_and_location();
      change_page(current_page,direction_and_location.first);
      cnt=0;
    }
    cnt++;

    draw_36hr_weather_forecast_on_canvas(off_screen_canvas,direction_and_location.second);    

    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
    off_screen_canvas->Clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));;  // Until Ctrl-C is pressed
  }
  
}


int main(){
  page_type current_page = CLOCK;
  rgb_matrix::RGBMatrix::Options defaults;
  rgb_matrix::RuntimeOptions runtime_opt;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.cols = 64;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = false;
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

  while(!interrupt_received){
    switch(current_page){
    case CLOCK:
      CLOCK_Page(matrix,current_page);
      break;
    case WEATHER_36HR_FORECAST:
      WEATHER_36HR_FORECAST_Page(matrix,current_page);
      break;
    }
  }

  clean_up();

}