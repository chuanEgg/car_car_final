#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <sqlite3.h>
#include <utility>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "weather.h"


class Database{
    public:
        //initiate connection with sqlite database
        Database(const char* name);

        //close connection with sqlite database
        ~Database();

        //get page_ctrl, page_activation, and location_ctrl , which are control parameters, from the database
        std::vector<int> get_control_parameters();

        //get the postal code, Chinese name, English name, and weather forecast code ID of a city from the database
        City get_city(int id);

        //update all weather data within object City from the database
        //Intended to be used for updating weather data in City object within the LED control thread
        //return 0 for success, 1 for failure
        int update_city_from_database(City &city);

        //update all weather data in sqlite database for a specified city using API, but doesn't edit data in City object
        //Intended to be used by API thread
        //return 0 for success, 1 for failure
        int update_database_from_API(City &city,const char* Authorization_code);

        
    private:
        sqlite3* db;
        const char* filename;

        //callback function for sqlite3
        static int callback_v1(void* data, int argc, char** argv, char** azColName);
        static int callback_v2(void* data, int argc, char** argv, char** azColName);
        static int callback_empty(void* NotUsed, int argc, char** argv, char** azColName);

        // return 0 for success, 1 for failure
        int update_City_Weather_36hr(City &city, WeatherData_days_per_12hr &weather_data,const char* Authorization_code);
        int update_City_Township_Weather_3_Days(City &city, WeatherData_days_per_12hr &weather_data, const char* Authorization_code);
};



#endif

