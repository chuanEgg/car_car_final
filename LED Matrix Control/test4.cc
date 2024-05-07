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

#include "modules.h"

/*
const char filename[20] = "/dev/i2c-1";
const char chipname[20] = "gpiochip0";
struct gpiod_chip *chip = nullptr;
*/
int main(){

    initialize_chip();

    /*
    BH1750 light_sensor(GND);
    light_sensor.change_mode(CONTINUOUSLY_H_RESOLUTION_MODE);
    for(int i = 0 ; i < 100 ; i++){
        std::cout<<light_sensor.get_lux()<<std::endl;
        usleep(100000);
    }
    light_sensor.~BH1750();
    */
    /*
    MMA8452 accelerometer(GND);
    for(int i = 0 ; i < 1000 ; i++){
        std::vector<int> acceleration = accelerometer.get_acceleration();
        std::cout<<acceleration[0]<<" "<<acceleration[1]<<" "<<acceleration[2]<<std::endl;
        usleep(100000);
    }
    */
    
    
    /*
    EC11E rotary_encoder(12,16,21);
    //rotary_encoder.get_rotation();
    
    for(int i = 0 ; i < 1000 ; i++){
        rotary_encoder.get_rotation();
        usleep(100000);
    }
    */

    SHT30 temperature_sensor(GND);
    for(int i = 0 ; i < 1000 ; i++){
        std::vector<double> vv = temperature_sensor.get_temperature_humidity();
        std::cout<<vv.at(0)<<' '<<vv.at(1)<<std::endl;
        usleep(100000);
    }
    close_chip();
    return 0;
}