#ifndef WEATHER_H
#define WEATHER_H

#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <sqlite3.h>

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
    WeatherData_days_per_12hr City_Township_Weather_3_Days
};


//PoP12->T->Wx->MinT_>MaxT
struct WeatherData_days_per_12hr {
    int size;
    std::vector<std::string> MaxT;
    std::vector<std::string> MinT;
    std::vector<std::string> T;
    std::vector<std::string> PoP12h;
    std::vector<std::string> Wx;
    std::vector<std::string> Time;
    std::vector<std::string> startTime;
    std::vector<std::string> endTime;
    
    long long int Time_Of_Last_Update_In_Seconds;
};

//callback function for curl
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

WeatherData_days_per_12hr get_city_township_weather_data(City &city);

WeatherData_days_per_12hr get_city_weather_data(City &city);



#endif