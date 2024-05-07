#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <sqlite3.h>
#include <utility>
#include <string>

struct City{
    int code;
    std::string Chinese_name_city;
    std::string Chinese_name_township;
    std::string English_name_city;
    std::string English_name_township;
    std::string Weather_Forecast_Code_2_Days;
    std::string Weather_Forecast_Code_7_Days;
};

class Database{
    public:
        Database(const char* name);
        ~Database();
        std::pair<int,int> get_direction_and_location();
        City get_city(int code);
        
    private:
        sqlite3* db;
        const char* filename;

        //callback function for sqlite3
        static int callback_v1(void* NotUsed, int argc, char** argv, char** azColName);
        static int callback_v2(void* NotUsed, int argc, char** argv, char** azColName);
        static int callback_v3(void* NotUsed, int argc, char** argv, char** azColName);
};



#endif

