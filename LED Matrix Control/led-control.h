#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <iostream>
#include <vector>

#include <math.h>
#include <signal.h>
#include <string>
#include <vector>
#include <exception>
#include <chrono>
#include <atomic>
#include <thread>

#include "draw_tools.h"
#include "database.h"



enum Page{
    Page0,
    Page1,
    Page2,
    Stop
};

Page int_to_page(int num);

class LED_Matrix{
    public:
        //initialize the LED matrix
        LED_Matrix(const char* font_filename);

        ~LED_Matrix();

        //function to change the page and pass in weather infomation
        // int page(Page page, std::atomic<int>& PAGE_FLAG, std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, City& city );
        
        // function to change the page and pass in weather infomation
        // If called first time, initialtes a thread for displaying a specific page, otherwise if change page while other pages are displaying, it ends the thread of displaying and start a new thread.
        int change_page(Page page, std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, const City& city );
    
    private:
        const char* time_font_filename;

        static inline std::atomic<bool> in_use;

        static inline std::thread led_thread;

        static inline rgb_matrix::RGBMatrix *matrix;
        static inline rgb_matrix::FrameCanvas * off_screen_canvas;
        static inline rgb_matrix::RGBMatrix::Options defaults;
        static inline rgb_matrix::RuntimeOptions runtime_opt;
        static inline rgb_matrix::Font time_font;

        static inline City city;

        static int page0(std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, const City& city );
        static int page1(std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, const City& city );
        static int page2(std::atomic<int>& temperature, std::atomic<int>& humidity, std::atomic<int>& lux, const City& city );
};

#endif // LED_CONTROL_H