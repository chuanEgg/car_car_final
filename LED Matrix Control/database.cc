#include "database.h"


Database::Database(const char* name){
    filename = name;
    int rc = sqlite3_open(filename, &db);
    if(rc){
        std::cerr<<"Can't open database: "<<sqlite3_errmsg(db)<<'\n';
    }
    else{
        std::cerr<<"Opened database successfully\n";
    }
}

Database::~Database(){
    sqlite3_close(db);
}

std::vector<int> Database::get_control_parameters(){    
    char* zErrMsg = 0;
    int rc;//sqlite execution return value, 0 for success
    const char* read_command = "SELECT page_ctrl,page_activation ,location_id_ctrl FROM Controls";
    const char* write_command = "UPDATE Controls set page_ctrl = 0 ";
    std::vector<int> output(3,0);
    output.at(0) = 0; //default value for moving 0 page forward or backward
    output.at(1) = -2147483648; //default value for page activation for all activated
    output.at(2) = 0; //default value for location id control corresponding to Taipei City Zongzen district


    rc = sqlite3_exec(db, read_command, callback_v1, &output, &zErrMsg);
    if( rc != SQLITE_OK ) {
        std::cerr<<"Failed to read control parameters from database\n"<<"SQL error: "<<zErrMsg<<std::endl;
        sqlite3_free(zErrMsg);
    }
    if(output.at(0) != 0){
        rc = sqlite3_exec(db, write_command, callback_empty, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            std::cerr<<"Failed to update control parameters from database\n"<<"SQL error: "<<zErrMsg<<std::endl;
            sqlite3_free(zErrMsg);
        }
    }

    return output;
}


City Database::get_city(int id){
    char* zErrMsg = 0;
    int rc;
    std::string command = "SELECT * FROM Location_Names WHERE id = " + std::to_string(id);
    const char* read_command = command.c_str();
    City output;

    rc = sqlite3_exec(db, read_command, callback_v2, &output, &zErrMsg);
    if( rc != SQLITE_OK ) {
        std::cerr<<"Failed to read city info from database\n"<<"SQL error: "<<zErrMsg<<'\n';
        sqlite3_free(zErrMsg);
    }

    return output;
}

int Database::update_database_from_API(City &city,const char* Authorization_code){
    WeatherData_days_per_12hr weather_data_36hr;
    WeatherData_days_per_12hr weather_data_3days;

    int rc = update_City_Weather_36hr(city, weather_data_36hr, Authorization_code);
    if(rc != 0){
        return 1;
    }


    return 0;
}

int update_City_Weather_36hr(City &city, WeatherData_days_per_12hr &weather_data,const char* Authorization_code){
    weather_data.size = 3;
    
    nlohmann::json weather_json_data;
    std::string readBuffer;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string url = "https://opendata.cwa.gov.tw/api/v1/rest/datastore/";
    std::string param0 = city.Weather_Forecast_Code_36_Hours_ID;
    std::string param1 = "Authorization=" + std::string(Authorization_code);
    std::string param2 = "format=JSON";
    std::string param3 = "locationName=" + city.Chinese_name_city;
    std::string total_url = url + param0 + "?" + param1 + "&" + param2 + "&" + param3;
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, total_url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if(res != CURLE_OK){
        std::cerr<<"Failed to retrieve data from weather API\n"<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;
        return 1;
      }
      else{
        weather_json_data = nlohmann::json::parse(readBuffer) ;
        for(int i = 0 ; i < weather_data.size ; i++){
            weather_data.MaxT.push_back( weather_json_data["records"]["location"][0]["weatherElement"][4]["time"][i]["parameter"]["parameterName"].get<std::string>() ) ;
            weather_data.MinT.push_back( weather_json_data["records"]["location"][0]["weatherElement"][2]["time"][i]["parameter"]["parameterName"].get<std::string>() );      
            weather_data.PoP12h.push_back( weather_json_data["records"]["location"][0]["weatherElement"][1]["time"][i]["parameter"]["parameterName"].get<std::string>() );     
            weather_data.Wx.push_back( weather_json_data["records"]["location"][0]["weatherElement"][0]["time"][i]["parameter"]["parameterName"].get<std::string>() );
            weather_data.startTime.push_back( weather_json_data["records"]["location"][0]["weatherElement"][0]["time"][i]["startTime"].get<std::string>() );
            weather_data.endTime.push_back( weather_json_data["records"]["location"][0]["weatherElement"][0]["time"][i]["endTime"].get<std::string>() );       
        }
      }
    }
    else{
        std::cerr<<"Failed to initiate curl\n";
        return 1;
    }
    
    
    // section to write data to database
    char* zErrMsg = 0;
    int rc;
    std::string write_command = "UPDATE City_Township_Weather_3_Days set ";

    for(int i = 0 ; i < city.City_Township_Weather_3_Days.size ; i++){
        write_command += "MaxT_" + std::to_string(i) + " = '" + weather_data.MaxT.at(i) + "', ";
        write_command += "MinT_" + std::to_string(i) + " = '" + weather_data.MinT.at(i) + "', ";
        write_command += "PoP12h_" + std::to_string(i) + " = '" + weather_data.PoP12h.at(i) + "', ";
        // write_command += "T_" + std::to_string(i) + " = '" + weather_data.T.at(i) + "', ";
        write_command += "Wx_" + std::to_string(i) + " = '" + weather_data.Wx.at(i) + "', ";
        // write_command += "Time_" + std::to_string(i) + " = '" + weather_data.Time.at(i) + "', ";
        write_command += "startTime_" + std::to_string(i) + " = '" + weather_data.startTime.at(i) + "', ";
        write_command += "endTime_" + std::to_string(i) + " = '" + weather_data.endTime.at(i) + "', ";
    }
    write_command += "WHERE id = " + std::to_string(city.id);
    const char* write_command_c = write_command.c_str();

    rc = sqlite3_exec(db, write_command_c, callback_empty, 0, &zErrMsg);
    if( rc != SQLITE_OK ) {
        std::cerr<<"Failed to update city weather info to database\n"<<"SQL error: "<<zErrMsg<<'\n';
        sqlite3_free(zErrMsg);
        return 1;
    }

    return 0;
}

int update_City_Township_Weather_3_Days(City &city, WeatherData_days_per_12hr &weather_data, const char* Authorization_code){
    WeatherData_days_per_12hr weather_data;
    weather_data.size = 6;
    
    nlohmann::json weather_json_data;
    std::string readBuffer;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string url = "https://opendata.cwa.gov.tw/api/v1/rest/datastore/";
    std::string param0 = city.Weather_Forecast_Code_7_Days_ID;
    std::string param1 = "Authorization=" + std::string(Authorization_code);
    std::string param2 = "format=JSON";
    std::string param3 = "locationName=" + city.Chinese_name_city;
    std::string total_url = url + param0 + "?" + param1 + "&" + param2 + "&" + param3;
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, total_url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if(res != CURLE_OK){
        std::cerr<<"Failed to retrieve data from weather API\n"<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;
        return 1;
      }
      else{
        weather_json_data = nlohmann::json::parse(readBuffer) ;
        for(int i = 0 ; i < weather_data.size ; i++){
            weather_data.MaxT.push_back( weather_json_data["records"]["location"][0]["weatherElement"][4]["time"][i]["parameter"]["parameterName"].get<std::string>() ) ;
            weather_data.MinT.push_back( weather_json_data["records"]["location"][0]["weatherElement"][2]["time"][i]["parameter"]["parameterName"].get<std::string>() );      
            weather_data.PoP12h.push_back( weather_json_data["records"]["location"][0]["weatherElement"][1]["time"][i]["parameter"]["parameterName"].get<std::string>() );            
        }
      }
    }
    else{
        std::cerr<<"Failed to initiate curl\n";
        return 1;
    }
    
    
    // section to write data to database
    char* zErrMsg = 0;
    int rc;
    std::string write_command = "UPDATE City_Township_Weather_3_Days set ";

    for(int i = 0 ; i < city.City_Township_Weather_3_Days.size ; i++){
        write_command += "MaxT_" + std::to_string(i) + " = '" + weather_data.MaxT.at(i) + "', ";
        write_command += "MinT_" + std::to_string(i) + " = '" + weather_data.MinT.at(i) + "', ";
        write_command += "PoP12h_" + std::to_string(i) + " = '" + weather_data.PoP12h.at(i) + "', ";
        // write_command += "T_" + std::to_string(i) + " = '" + weather_data.T.at(i) + "', ";
        write_command += "Wx_" + std::to_string(i) + " = '" + weather_data.Wx.at(i) + "', ";
        // write_command += "Time_" + std::to_string(i) + " = '" + weather_data.Time.at(i) + "', ";
        write_command += "startTime_" + std::to_string(i) + " = '" + weather_data.startTime.at(i) + "', ";
        write_command += "endTime_" + std::to_string(i) + " = '" + weather_data.endTime.at(i) + "', ";
    }
    write_command += "WHERE id = " + std::to_string(city.id);
    const char* write_command_c = write_command.c_str();

    rc = sqlite3_exec(db, write_command_c, callback_empty, 0, &zErrMsg);
    if( rc != SQLITE_OK ) {
        std::cerr<<"Failed to update city weather info to database\n"<<"SQL error: "<<zErrMsg<<'\n';
        sqlite3_free(zErrMsg);
        return 1;
    }

    return 0;
}





int Database::callback_v1(void* data, int argc, char** argv, char** azColName) {
    std::vector<int>* result = (std::vector<int>*)data;
    if(argc != 3){
        std::cerr<<"Error: callback function callback_v1 received wrong number of arguments\n";
        return 1;
    }
    result->at(0) = atoi(argv[0]);
    result->at(1) = atoi(argv[1]);
    result->at(2) = atoi(argv[2]);
    return 0;
}


int Database::callback_v2(void* data, int argc, char** argv, char** azColName) {
    City* result = (City*)data;
    if(argc != 7){
        std::cerr<<"Error: callback function callback_v3 received wrong number of arguments\n";
        return 1;
    }
    result->id = atoi(argv[0]);
    result->postal_code = atoi(argv[1]);
    std::string Chinese_name = argv[2];
    std::string English_name = argv[3];

    result->Weather_Forecast_Code_36_Hours_ID = argv[4];
    result->Weather_Forecast_Code_2_Days_ID = argv[5];
    result->Weather_Forecast_Code_7_Days_ID = argv[6];

    result->Chinese_name_city = Chinese_name.substr(0,9);
    result->Chinese_name_township = Chinese_name.substr(9);
    // std::cout<<Chinese_name<<' '<<result->Chinese_name_city<<' '<<result->Chinese_name_township<<std::endl;

    result->English_name_city = English_name.substr(English_name.find_first_of(',')+2);
    result->English_name_township = English_name.substr(0, English_name.find_first_of(','));

    return 0;
}

int Database::callback_empty(void* NotUsed, int argc, char** argv, char** azColName) {
    return 0;
}
