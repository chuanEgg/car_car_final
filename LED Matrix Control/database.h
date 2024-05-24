#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <sqlite3.h>
#include <utility>
#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

// #include "weather.h"

struct Control{
    int page_ctrl;
    int page_activation;
    int location_ctrl;
};


// Wx,PoP12,MinT,MaxT,startTime,endTime
struct WeatherData_days_per_12hr {
    int size;
    std::vector<std::string> Wx;
    std::vector<std::string> PoP12h;
    std::vector<std::string> MinT;
    std::vector<std::string> MaxT;
    std::vector<std::string> startTime;
    std::vector<std::string> endTime;
    
    long long int Time_Of_Last_Update_In_Seconds;
};

struct City{
    int id;
    int postal_code;
    std::string Chinese_name_city;
    std::string Chinese_name_township;
    std::string English_name_city;
    std::string English_name_township;
    std::string Weather_Forecast_Code_36_Hours_ID;
    std::string Weather_Forecast_Code_2_Days_ID;
    std::string Weather_Forecast_Code_7_Days_ID;//using this one for 3 days weather forecast

    WeatherData_days_per_12hr City_Weather_36hr;
    WeatherData_days_per_12hr City_Township_Weather_3_Days;
};



class Database{
    public:
        //initiate connection with sqlite database
        Database(const char* filename);

        //close connection with sqlite database
        ~Database();

        //get control parameters from the database
        Control get_control_parameters();

        //get the id, postal code, Chinese name, English name, and weather forecast code ID of a city from the database
        City get_city(int id);

        //update all weather data within object City from the database
        //Intended to be used for updating weather data in City object within the LED control thread
        //return 0 for success, 1 for failure
        int update_city_from_database(City &city);

        //update all weather data in sqlite database for a specified city using API, but doesn't edit data in City object
        //Intended to be used by API thread
        //return 0 for success, 1 for failure
        int update_database_from_API(const City &city,const char* Authorization_code);

        
    private:
        sqlite3* db;
        const char* filename;

        //callback function for sqlite3
        static int callback_v1(void* data, int argc, char** argv, char** azColName);
        static int callback_v2(void* data, int argc, char** argv, char** azColName);
        static int callback_v3(void* data, int argc, char** argv, char** azColName);
        static int callback_v4(void* data, int argc, char** argv, char** azColName);
        static int callback_empty(void* NotUsed, int argc, char** argv, char** azColName);

        // return 0 for success, 1 for failure
        int update_City_Weather_36hr(const City &city, const char* Authorization_code);
        int update_City_Township_Weather_3_Days(const City &city, const char* Authorization_code);

        // callback function for curl
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
};



#endif

