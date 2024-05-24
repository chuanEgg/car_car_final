#include <iostream>
#include <stdio.h>

#include "include/led-matrix.h"
#include "include/graphics.h"
#include "modules.h"
#include "database.h"

#include <cmath>
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
#include <atomic>

#include <fstream>

enum page_type{
    START_UP = 0,
    P1_CLOCK = 1,
    P2_WEATHER = 2
};

page_type int_to_page_type(int num){
    switch(num){
        case 0:
            return page_type::START_UP;
        case 1:
            return page_type::P1_CLOCK;
        case 2:
            return page_type::P2_WEATHER;
        default:
            std::cerr<<"Invalid page number"<<std::endl;
            return page_type::START_UP;
    }
}


volatile bool interrupt_received = false;
struct gpiod_chip *chip = nullptr;

const char* filenames[1] = {"../images/sakura.gif"};
const char* time_font_filename = "../fonts/5x7.bdf";
const char* database_filename = "../../SQLite Database/database.db";
const char* api_key_filename = "../api_key/api_key.txt";
const char* api_key;

std::atomic<int> SHT30_temperature_x10(0), SHT30_humidity_x10(0), BH1750_lux(0);
std::atomic<bool> sensor_thread_control(true);//1 for running, 0 for stopping

void api_thread_function(const City& city, const char* api_key){
    Database database(database_filename);
    if( database.update_database_from_API(city, api_key) ){
        std::cerr<<"Failed to update database from API"<<std::endl;
    }
}

void sensor_thread_function(){
    SHT30 sht30(GND);
    BH1750 bh1750(GND);
    bh1750.change_mode(ONE_TIME_L_RESOLUTION_MODE);

    std::vector<double> temp_humidity;
    while(sensor_thread_control){
        temp_humidity = sht30.get_temperature_humidity();
        SHT30_temperature_x10 = (int)(round(temp_humidity.at(0)*10));
        SHT30_humidity_x10 = (int)(round(temp_humidity.at(1)*10));
        BH1750_lux = bh1750.get_lux();
        usleep(1000000);
    }
}

int get_api_key(const char* filename){
    std::ifstream file(filename);  // Open the file

    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }
    std::string api_key_str;
    char ch;
    while (file.get(ch)) {  // Read each character into 'ch'
        if(ch >= 33 && ch <= 126)//only read printable characters
            api_key_str += ch;
        else{break;}
        
    }
    int sz = api_key_str.size()+1;
    char* temp = new char[sz];
    for(int i = 0 ; i < sz-1 ; i++){
        temp[i] = api_key_str.at(i);
    }
    temp[sz-1] = '\0';
    api_key = temp;
    file.close();  // Close the file
    return 0;
}

int main(){

    if(get_api_key(api_key_filename)){
        std::cerr<<"Failed to get API key"<<std::endl;
        return 1;
    }
    std::cout<<api_key<<std::endl;
    

    initialize_chip();


    std::thread sensor_thread(sensor_thread_function);// initiate sensor thread

    Database database(database_filename);
    Control control = database.get_control_parameters();
    page_type page = int_to_page_type(control.page_ctrl);
    City city = database.get_city(control.location_ctrl);
    



    //test
    std::thread api_thread(api_thread_function, city, api_key);
    std::cout<<"API thread started\n";
    
    api_thread.join();
    std::cout<<"API thread joined\n";

    std::cout<<"about to update city from database\n";
    database.update_city_from_database(city);   
    std::cout<<"city updated from database\n";

    int n = 0;
    while(n<15){
        std::cout<<(double)(SHT30_temperature_x10)/10<<' '<<(double)(SHT30_humidity_x10)/10<<' '<<BH1750_lux<<std::endl;
        std::cout<<city.Chinese_name_city<<' '<<city.Chinese_name_township<<std::endl;
        std::cout<<city.English_name_city<<' '<<city.English_name_township<<std::endl;
        std::cout<<city.City_Weather_36hr.PoP12h.at(0)<<std::endl;
        std::cout<<city.City_Weather_36hr.Time_Of_Last_Update_In_Seconds<<std::endl;
        if(n%3==0){
            control.location_ctrl++;
            city = database.get_city(control.location_ctrl);
            api_thread = std::thread(api_thread_function, city, api_key);
            
        }

        if(api_thread.joinable()){
            api_thread.join();
            std::cout<<"API thread joined\n";
            database.update_city_from_database(city);
        }        
        usleep(1000000);
        n++;
    }

    //test end


    // while(!interrupt_received){
    //     switch(page){
    //         case START_UP:
    //             break;
    //         case P1_CLOCK:
    //             break;
    //         case P2_WEATHER:
    //             break;
    //     }
    // }

    sensor_thread_control = false;
    sensor_thread.join();
    close_chip();
    
}