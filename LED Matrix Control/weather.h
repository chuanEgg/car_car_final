#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "database.h"


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
    
};

//callback function for curl
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

WeatherData_days_per_12hr get_city_township_weather_data(City &city);

WeatherData_days_per_12hr get_city_weather_data(City &city);