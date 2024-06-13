#include "led-control.h"

Page int_to_page(int num){
    num = (num >= 0) ? num : 3 + num % 3 ;
    switch(num%3){
        case 0:
            return Page::Page0;
        case 1:
            return Page::Page1;
        case 2:
            return Page::Page2;
        default:
            std::cerr<<"Invalid page number"<<std::endl;
            return Page::Page0;
    }
}

LED_Matrix::LED_Matrix(const char* font_filename):time_font_filename(font_filename){
    in_use = false;

    // Set up the RGBMatrix. This is based on the example code provided by Henner Zeller
    // Set up the parameters that match the LED Matrix that we're using, which is 64*32
    // defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
    defaults.rows = 32;
    defaults.cols = 64;
    defaults.chain_length = 1;
    defaults.parallel = 1;
    defaults.show_refresh_rate = false;
    defaults.limit_refresh_rate_hz = 300;
    defaults.disable_hardware_pulsing = false;
    defaults.pwm_lsb_nanoseconds = 200;
    defaults.scan_mode = 0;
    // defaults.hardware_mapping = "custom";
    defaults.hardware_mapping = "custom for testing";
    

    runtime_opt.gpio_slowdown = 2;

    // Initialize the RGBMatrix object
    matrix = rgb_matrix::RGBMatrix::CreateFromOptions(defaults, runtime_opt);
    // Initialize the off screen canvas
    off_screen_canvas = matrix->CreateFrameCanvas();

    //initialize the font
    Magick::InitializeMagick(nullptr);
    if (!time_font.LoadFont(time_font_filename)) {
        std::cerr<< "Couldn't load font "<<time_font_filename<<'\n';
    }
  
}

LED_Matrix::~LED_Matrix(){
    if(in_use == true){
        if(led_thread.joinable()){
            led_thread.join();
        }
    }
}

/*
int LED_Matrix::page(Page page, std::atomic<int>& PAGE_FLAG, std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, City& city ){
    if(in_use == true){
        std::cerr<<"LED Matrix is in use by others\n";
        return 1;
    }
    else if(PAGE_FLAG == false){
        std::cerr<<"LED Matrix PAGE_FLAG is false, failed to start LED matrix\n";
        return 1;
    }
    int ret = 0;
    switch(page){
        case Page0:
            in_use = true;
            ret = page0(PAGE_FLAG, temperature, humidity, lux, city);
            in_use = false;
            return ret;
        case Page1:
            in_use = true;
            ret = page1(PAGE_FLAG, temperature, humidity, lux, city);
            in_use = false;
            return ret;
        case Page2:
            in_use = true;
            ret = page2(PAGE_FLAG, temperature, humidity, lux, city);
            in_use = false;
            return ret;
        default:
            std::cerr<<"Invalid page number\n";
            return 1;
    }
}
*/

int LED_Matrix::change_page(Page page, std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, const City& city ){
    if(in_use == true){
        in_use = false;
        if(led_thread.joinable()){
            led_thread.join();
        }
        else{
            std::cerr<<"Failed to join the thread\n";
            return 1;
        }
    }

    std::cout<<"In Changing page\n";
    
    switch(page){
        case Page0:
            in_use = true;
            led_thread = std::thread(page0, std::ref(temperature), std::ref(humidity), std::ref(lux), city);
            break;
        case Page1:
            in_use = true;
            led_thread = std::thread(page1, std::ref(temperature), std::ref(humidity), std::ref(lux), city);
            break;
        case Page2:
            in_use = true;
            led_thread = std::thread(page2, std::ref(temperature), std::ref(humidity), std::ref(lux), city);
            break;
        case Stop:
            std::cout<<"Stopped page display\n";
            return 0;
        default:
            std::cerr<<"Invalid page number\n";
            return 1;
    }
    return 0;
}


int LED_Matrix::page0(std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, const City& city ){
    std::string time_str;
    std::string weather_str;
    std::string temperature_str;
    std::string humidity_str;
    std::string lux_str;
    std::string temp_and_humidity_str;

    int framerate = 30;
    
    auto last_time = std::chrono::system_clock::now();
    while(in_use){
        // Get the current time
        // Get the current time_point from the system clock
        auto now = std::chrono::system_clock::now();

        // Convert time_point to time_t to make it easier to extract time information
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

        // Convert time_t to tm as a local time
        std::tm now_tm = *std::localtime(&now_time_t);

        // Create the time string
        if(now_tm.tm_mon<10){
            time_str = '0' + std::to_string(now_tm.tm_mon) + '/';
        }
        else{
            time_str = std::to_string(now_tm.tm_mon) + '/';
        }

        if(now_tm.tm_mday<10){
            time_str += '0' + std::to_string(now_tm.tm_mday) + ' ';
        }
        else{
            time_str += std::to_string(now_tm.tm_mday) + ' ';
        }

        if(now_tm.tm_hour<10){
            time_str += '0' + std::to_string(now_tm.tm_hour) + ':';
        }
        else{
            time_str += std::to_string(now_tm.tm_hour) + ':';
        }

        if(now_tm.tm_min<10){
            time_str += '0' + std::to_string(now_tm.tm_min);
        }
        else{
            time_str += std::to_string(now_tm.tm_min);
        }


        // Get the current weather information
        int temperature_int = temperature.load();

        temperature_str = std::to_string(temperature_int/10) + '.' + std::to_string(temperature_int%10) + "C";
        humidity_str = std::to_string(humidity.load()/10) + "%";
        lux_str = std::to_string(lux.load()) + "lux";
        temp_and_humidity_str = temperature_str + " " + humidity_str;

        // Draw the time
        //centered at the top of the screen
        rgb_matrix::DrawText(off_screen_canvas, time_font, ( off_screen_canvas->width() - time_str.length()*5)/2 , time_font.baseline()*1, rgb_matrix::Color(0, 100, 255), time_str.c_str());

        // Draw the weather information
        // rgb_matrix::DrawText(off_screen_canvas, time_font, 0, time_font.baseline()*2, rgb_matrix::Color(255, 255, 255), temperature_str.c_str());
        // rgb_matrix::DrawText(off_screen_canvas, time_font, 0, time_font.baseline()*3, rgb_matrix::Color(255, 0, 255), humidity_str.c_str());
        // rgb_matrix::DrawText(off_screen_canvas, time_font, 0, time_font.baseline()*4, rgb_matrix::Color(255, 255, 255), lux_str.c_str());

        // draw the temperature and humidity
        rgb_matrix::DrawText(off_screen_canvas, time_font, ( off_screen_canvas->width() - temp_and_humidity_str.length()*5)/2, time_font.baseline()*2+1, rgb_matrix::Color(0, 255, 0), temp_and_humidity_str.c_str());

        // Draw the weather information
        // Drawing as ticker
        static double precise_x = off_screen_canvas->width() + 5;
        static double precise_y = time_font.baseline()*3 + 2;
        static int letter_spacing = 0;
        static double speed = 0.8;

        int x = (int)round(precise_x);
        int y = (int)round(precise_y);
        std::string line =  city.English_name_township + ", " + city.English_name_city +
            "  Temp " + city.City_Township_Weather_3_Days.MaxT.at(0) + " - " + city.City_Township_Weather_3_Days.MinT.at(0) + "C" + 
            "  Rain " + city.City_Township_Weather_3_Days.PoP12h.at(0) + "%";

        int length = rgb_matrix::DrawText(off_screen_canvas, time_font , x, y + time_font.baseline(), rgb_matrix::Color(0, 100, 255), NULL, line.c_str(), 0);
        precise_x -= speed;
        if(x + length < 0){
            precise_x = off_screen_canvas->width() + 5;
        }
        // end of draw weather information

        


        // Update the display
        off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
        // Clear the canvas for next draw
        off_screen_canvas->Clear();

        // to maintain visual framerate
        if((   std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_time).count() < 1000/framerate)){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/framerate) - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_time));
        }
        last_time = std::chrono::system_clock::now();

    }
    // Ensure that upon ending every page, the screen is cleared
    off_screen_canvas->Clear();
    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
    
    return 0;
}

int LED_Matrix::page1(std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, const City& city ){
    std::string date_str;
    std::string time_str;
    std::string weather_str;
    std::string temperature_str;
    std::string humidity_str;
    std::string lux_str;
    std::string temp_and_humidity_str;
    std::vector<std::string> weather_str_list;

    int framerate = 20;
    int frame_num = 0;
    
    auto last_time = std::chrono::system_clock::now();//for maintaining visual framerate
    
    ImageVector images = LoadImageAndScaleImage("../images/rem_32.gif",24, 24);
    int cnt = 0;
    while(in_use){
        cnt++;
        // Get the current time
        // Get the current time_point from the system clock
        auto now = std::chrono::system_clock::now();

        // draw rem on ipper left corner with 24*24 size
        CopyImageToCanvas(images.at(frame_num), off_screen_canvas, 0, 0);
        if(cnt%4==0){
            frame_num += 1;
            frame_num = frame_num >= (int)images.size() ? 0 : frame_num;
        }          

        // Convert time_point to time_t to make it easier to extract time information
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

        // Convert time_t to tm as a local time
        std::tm now_tm = *std::localtime(&now_time_t);

        

        // Create the date and time string
        if(now_tm.tm_mon<10){
            date_str = '0' + std::to_string(now_tm.tm_mon) + '/';
        }
        else{
            date_str = std::to_string(now_tm.tm_mon) + '/';
        }

        if(now_tm.tm_mday<10){
            date_str += '0' + std::to_string(now_tm.tm_mday) + ' ';
        }
        else{
            date_str += std::to_string(now_tm.tm_mday) + ' ';
        }

        if(now_tm.tm_hour<10){
            time_str = '0' + std::to_string(now_tm.tm_hour) + ':';
        }
        else{
            time_str = std::to_string(now_tm.tm_hour) + ':';
        }

        if(now_tm.tm_min<10){
            time_str += '0' + std::to_string(now_tm.tm_min);
        }
        else{
            time_str += std::to_string(now_tm.tm_min);
        }


        temperature_str = std::to_string(temperature.load()/10) + '.' + std::to_string(temperature.load()%10);
        humidity_str = std::to_string(humidity.load()/10) + "%";
        lux_str = std::to_string(lux.load()) + "lux";
        temp_and_humidity_str = temperature_str + " " + humidity_str;

        // Draw the date
        //at the top right of the screen
        rgb_matrix::DrawText(off_screen_canvas, time_font, 25 , time_font.baseline()*1, rgb_matrix::Color(0, 100, 255), date_str.c_str());
        // Draw the time
        //at the top right of the screen
        rgb_matrix::DrawText(off_screen_canvas, time_font, 25 , time_font.baseline()*2 + 1, rgb_matrix::Color(0, 100, 255), time_str.c_str());

        // draw the temperature and humidity at the right of screen
        rgb_matrix::DrawText(off_screen_canvas, time_font, 25, time_font.baseline()*3 + 2, rgb_matrix::Color(0, 255, 0), temp_and_humidity_str.c_str());
        
        // Draw the weather information
        // Drawing as ticker
        static double precise_x = off_screen_canvas->width() + 5;
        static double precise_y = time_font.baseline()*4+1;
        static int letter_spacing = 0;
        static double speed = 0.8;

        int x = (int)round(precise_x);
        int y = (int)round(precise_y);
        std::string line =  city.English_name_township + ", " + city.English_name_city +
            "  Temp " + city.City_Township_Weather_3_Days.MaxT.at(0) + " - " + city.City_Township_Weather_3_Days.MinT.at(0) + "C" + 
            "  Rain " + city.City_Township_Weather_3_Days.PoP12h.at(0) + "%";

        int length = rgb_matrix::DrawText(off_screen_canvas, time_font , x, y + time_font.baseline(), rgb_matrix::Color(0, 100, 255), NULL, line.c_str(), 0);
        precise_x -= speed;
        if(x + length < 0){
            precise_x = off_screen_canvas->width() + 5;
        }
        // end of draw weather information

        


        // Update the display
        off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
        // Clear the canvas for next draw
        off_screen_canvas->Clear();

        // to maintain visual framerate
        if((   std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_time).count() < 1000/framerate)){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/framerate) - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_time));
        }
        last_time = std::chrono::system_clock::now();

    }
    // Ensure that upon ending every page, the screen is cleared
    off_screen_canvas->Clear();
    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
    return 0;
}

int LED_Matrix::page2(std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, const City& city ){
    int frame_num = 0;
    int framerate = 5;
    auto last_time = std::chrono::system_clock::now();//for maintaining visual framerate
    
    ImageVector images = LoadImageAndScaleImage("../images/rem_32.gif", 32, 32);
    while(in_use){

        CopyImageToCanvas(images.at(frame_num), off_screen_canvas, 0, 0);
        CopyImageToCanvas(images.at(frame_num), off_screen_canvas, 32, 0);
        frame_num += 1;
        frame_num = frame_num >= (int)images.size() ? 0 : frame_num;
                    
        // Update the display
        off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
        // Clear the canvas for next draw
        off_screen_canvas->Clear();

        if((   std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_time).count() < 1000/framerate)){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/framerate) - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_time));
        }
        last_time = std::chrono::system_clock::now();

    }
    // Ensure that upon ending every page, the screen is cleared
    off_screen_canvas->Clear();
    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
    return 0;
}



