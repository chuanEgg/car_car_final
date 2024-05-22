#include <iostream>
#include <stdio.h>

#include "include/led-matrix.h"
#include "include/graphics.h"
#include "modules.h"

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

#include <thread>



volatile bool interrupt_received = false;
struct gpiod_chip *chip = nullptr;


void update_weather_to_database(){}

void update_sensors_to_database(){}

int main(){
    
    initialize_chip();

    close_chip();
    
}