#include "database.h"


Database::Database(const char* filename){
    int rc = sqlite3_open(filename, &db);
    if(rc){
        std::cerr<<"Can't open database: "<<sqlite3_errmsg(db)<<'\n';
    }
}

Database::~Database(){
    sqlite3_close(db);
}

Control Database::get_control_parameters(){    
    char* zErrMsg = 0;
    int rc;//sqlite execution return value, 0 for success
    const char* read_command = "SELECT page_ctrl, page_activation, location_id_ctrl FROM Controls";
    const char* write_command = "UPDATE Controls set page_ctrl = 0 ";
    Control output;
    output.page_ctrl = 0; //default value for moving 0 page forward or backward
    output.page_activation = -2147483648; //default value for page activation for all activated 11111111 11111111 11111111 11111111 (maximum 32 pages)
    output.location_ctrl = 0; //default value for location id control corresponding to first one on database


    rc = sqlite3_exec(db, read_command, callback_v1, &output, &zErrMsg);
    if( rc != SQLITE_OK ) {
        std::cerr<<"Failed to read control parameters from database\n"<<"SQL error: "<<zErrMsg<<std::endl;
        sqlite3_free(zErrMsg);
    }
    if(output.page_ctrl != 0){
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

int Database::update_city_from_database(City &city){
    WeatherData_days_per_12hr weather_data_36hr;
    WeatherData_days_per_12hr weather_data_3days;
    weather_data_36hr.size = 3;
    weather_data_3days.size = 6;
    
    char* zErrMsg1;
    char* zErrMsg2;
    int rc1, rc2;
    // std::cout<<"city id: "<<city.id<<std::endl;
    std::string command = "SELECT * FROM City_Weather_36hr WHERE id = " + std::to_string(city.id);
    const char* read_command = command.c_str();
    
    rc1 = sqlite3_exec(db, read_command, callback_v3, &city, &zErrMsg1);
    if( rc1 != SQLITE_OK ) {
        std::cerr<<"Failed to read City_Weather_36hr info from database\n"<<"SQL error: "<<zErrMsg1<<'\n';
        sqlite3_free(zErrMsg1);
        return 1;
    }

    command = "SELECT * FROM City_Township_Weather_3_Days WHERE id = " + std::to_string(city.id);
    read_command = command.c_str();
    rc2 = sqlite3_exec(db, read_command, callback_v4, &city, &zErrMsg2);
    if( rc2 != SQLITE_OK ) {
        std::cerr<<"Failed to read City_Township_Weather_3_Days info from database\n"<<"SQL error: "<<zErrMsg2<<'\n';
        sqlite3_free(zErrMsg2);
        return 1;
    }
    return 0;
}

int Database::update_database_from_API(const City &city,const char* Authorization_code){
    int rc1 = update_City_Weather_36hr(city,Authorization_code);
    int rc2 = update_City_Township_Weather_3_Days(city, Authorization_code);
    if(rc1+rc2 != 0){
        return 1;
    }
    return 0;
}

int Database::update_City_Weather_36hr(const City &city, const char* Authorization_code){
    WeatherData_days_per_12hr weather_data;
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
    std::string param4 = "elementName=Wx,PoP,MinT,MaxT";// it's PoP not PoP12h
    std::string total_url = url + param0 + "?" + param1 + "&" + param2 + "&" + param3 + "&" + param4;

    // std::cout<<total_url<<std::endl;

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
      else if(readBuffer.empty()){
        std::cerr<<"Failed to retrieve data from weather API\n"<<"Empty data received\n";
        return 1;
      }
      else{
        // std::cout<<"to read buffer: "<<readBuffer<<std::endl;
        weather_json_data = nlohmann::json::parse(readBuffer) ;
        std::vector<int> checklist(4,0); // to check the order of data to match the order of data below
        for(int k = 0 ; k < 4 ; k++){
            std::string temp = weather_json_data["records"]["location"][0]["weatherElement"][k]["elementName"].get<std::string>();
            if(temp.compare("Wx") == 0){
                checklist.at(0) = k;
            }
            else if(temp.compare("PoP") == 0){
                checklist.at(1) = k;
            }
            else if(temp.compare("MinT") == 0){
                checklist.at(2) = k;
            }
            else if(temp.compare("MaxT") == 0){
                checklist.at(3) = k;
            }
            else{
                std::cerr<<"Error: unexpected elementName from API\n";
                return 1;
            }
        }
        for(int i = 0 ; i < weather_data.size ; i++){
            weather_data.Wx.push_back( weather_json_data["records"]["location"][0]["weatherElement"][checklist.at(0)]["time"][i]["parameter"]["parameterValue"].get<std::string>() );
            weather_data.PoP12h.push_back( weather_json_data["records"]["location"][0]["weatherElement"][checklist.at(1)]["time"][i]["parameter"]["parameterName"].get<std::string>() );     
            weather_data.MinT.push_back( weather_json_data["records"]["location"][0]["weatherElement"][checklist.at(2)]["time"][i]["parameter"]["parameterName"].get<std::string>() );      
            weather_data.MaxT.push_back( weather_json_data["records"]["location"][0]["weatherElement"][checklist.at(3)]["time"][i]["parameter"]["parameterName"].get<std::string>() ) ;

            weather_data.startTime.push_back( weather_json_data["records"]["location"][0]["weatherElement"][checklist.at(0)]["time"][i]["startTime"].get<std::string>() );
            weather_data.endTime.push_back( weather_json_data["records"]["location"][0]["weatherElement"][checklist.at(0)]["time"][i]["endTime"].get<std::string>() );       
            // std::cout<<"here:"<<i<<std::endl;
        }
        weather_data.Time_Of_Last_Update_In_Seconds = (long long int)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      }
    }
    else{
        std::cerr<<"Failed to initiate curl\n";
        return 1;
    }
    
    
    // section to write data to database
    char* zErrMsg = 0;
    int rc;
    std::string write_command = "UPDATE City_Weather_36hr SET ";

    for(int i = 0 ; i < weather_data.size ; i++){
        write_command += "MaxT_12hr_" + std::to_string(i) + " = '" + weather_data.MaxT.at(i) + "', ";
        write_command += "MinT_12hr_" + std::to_string(i) + " = '" + weather_data.MinT.at(i) + "', ";
        write_command += "PoP_12hr_" + std::to_string(i) + " = '" + weather_data.PoP12h.at(i) + "', ";
        write_command += "Wx_12hr_" + std::to_string(i) + " = '" + weather_data.Wx.at(i) + "', ";
        write_command += "startTime_" + std::to_string(i) + " = '" + weather_data.startTime.at(i) + "', ";
        write_command += "endTime_" + std::to_string(i) + " = '" + weather_data.endTime.at(i) + "', ";
    }
    write_command += "Time_Of_Last_Update_In_Seconds = " + std::to_string(weather_data.Time_Of_Last_Update_In_Seconds) + ' ';
    write_command += "WHERE id = " + std::to_string(city.id);
    const char* write_command_c = write_command.c_str();
    // std::cout<<write_command<<std::endl;

    rc = sqlite3_exec(db, write_command_c, callback_empty, 0, &zErrMsg);
    if( rc != SQLITE_OK ) {
        std::cerr<<"Failed to update city weather info to database\n"<<"SQL error: "<<zErrMsg<<'\n';
        sqlite3_free(zErrMsg);
        return 1;
    }

    // std::cout<<"update city weather 36hr success\n";

    return 0;
}

int Database::update_City_Township_Weather_3_Days(const City &city, const char* Authorization_code){
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
    std::string param3 = "locationName=" + city.Chinese_name_township;
    std::string param4 = "elementName=Wx,PoP12h,MinT,MaxT";
    std::string total_url = url + param0 + "?" + param1 + "&" + param2 + "&" + param3 + "&" + param4;
    // std::cout<<total_url<<std::endl;
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
    else if(readBuffer.empty()){
        std::cerr<<"Failed to retrieve data from weather API\n"<<"Empty data received\n";
        return 1;
      }
      else{
        // std::cout<<"to read buffer: "<<readBuffer<<std::endl;
        weather_json_data = nlohmann::json::parse(readBuffer) ;
        std::vector<int> checklist(4,0); // to check the order of data to match the order of data below
        for(int k = 0 ; k < 4 ; k++){
            std::string temp = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][k]["elementName"].get<std::string>();
            // std::cout<<"3 day get element name success\n";
            if(temp.compare("Wx") == 0){
                checklist.at(0) = k;
            }
            else if(temp.compare("PoP12h") == 0){
                checklist.at(1) = k;
            }
            else if(temp.compare("MinT") == 0){
                checklist.at(2) = k;
            }
            else if(temp.compare("MaxT") == 0){
                checklist.at(3) = k;
            }
            else{
                std::cerr<<"Error: unexpected elementName from API\n";
                return 1;
            }
        }
        for(int i = 0 ; i < weather_data.size ; i++){
            weather_data.Wx.push_back( weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][checklist.at(0)]["time"][i]["elementValue"][1]["value"].get<std::string>() );
            weather_data.PoP12h.push_back( weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][checklist.at(1)]["time"][i]["elementValue"][0]["value"].get<std::string>() );     
            weather_data.MinT.push_back( weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][checklist.at(2)]["time"][i]["elementValue"][0]["value"].get<std::string>() );      
            weather_data.MaxT.push_back( weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][checklist.at(3)]["time"][i]["elementValue"][0]["value"].get<std::string>() ) ;

            weather_data.startTime.push_back( weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][checklist.at(0)]["time"][i]["startTime"].get<std::string>() );
            weather_data.endTime.push_back( weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][checklist.at(0)]["time"][i]["endTime"].get<std::string>() );       
            // std::cout<<"here:"<<i<<std::endl;
        }
        weather_data.Time_Of_Last_Update_In_Seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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

    for(int i = 0 ; i < weather_data.size ; i++){
        write_command += "MaxT_12hr_" + std::to_string(i) + " = '" + weather_data.MaxT.at(i) + "', ";
        write_command += "MinT_12hr_" + std::to_string(i) + " = '" + weather_data.MinT.at(i) + "', ";
        write_command += "PoP_12hr_" + std::to_string(i) + " = '" + weather_data.PoP12h.at(i) + "', ";
        write_command += "Wx_12hr_" + std::to_string(i) + " = '" + weather_data.Wx.at(i) + "', ";
        write_command += "startTime_" + std::to_string(i) + " = '" + weather_data.startTime.at(i) + "', ";
        write_command += "endTime_" + std::to_string(i) + " = '" + weather_data.endTime.at(i) + "', ";
    }
    write_command += "Time_Of_Last_Update_In_Seconds = " + std::to_string(weather_data.Time_Of_Last_Update_In_Seconds) + ' ';
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
    Control* result = (Control*)data;
    if(argc != 3){
        std::cerr<<"Error: callback function callback_v1 received wrong number of arguments\n";
        return 1;
    }
    result->page_ctrl = atoi(argv[0]);
    result->page_activation = atoi(argv[1]);
    result->location_ctrl = atoi(argv[2]);
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

int Database::callback_v3(void* data, int argc, char** argv, char** azColName){
    City* city = (City*)data;
    WeatherData_days_per_12hr weather_data_36hr;
    weather_data_36hr.size = 3;
    // std::cout<<"callback_v3\n";
    

    if(argc != 23){
        std::cerr<<"Error: callback function callback_v3 received wrong number of arguments\n";
        return 1;
    }
    // std::cout<<'\n'<<argc<<std::endl;  
    // for(int i = 0 ; i < argc ; i++){
    //     std::cout<<argv[i]<<' ';
    // }
    

    int pos = 4;
    for( int i = 0 ; i < weather_data_36hr.size ; i++){
        weather_data_36hr.Wx.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_36hr.size ; i++){
        weather_data_36hr.PoP12h.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_36hr.size ; i++){
        weather_data_36hr.MinT.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_36hr.size ; i++){
        weather_data_36hr.MaxT.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_36hr.size ; i++){
        weather_data_36hr.startTime.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_36hr.size ; i++){
        weather_data_36hr.endTime.push_back(std::string(argv[pos]));
        pos++;
    }

    // std::cout<<"callback_v3 almost success\n";
    city->City_Weather_36hr = weather_data_36hr;
    // std::cout<<"callback_v3 success\n";

    return 0;
}

int Database::callback_v4(void* data, int argc, char** argv, char** azColName){
    City* city = (City*)data;
    WeatherData_days_per_12hr weather_data_3days;
    weather_data_3days.size = 6;

    if(argc != 41){
        std::cerr<<"Error: callback function callback_v3 received wrong number of arguments\n";
        return 1;
    }

    std::vector<std::string> argv_string(argc);
    for(int i = 0 ; i < argc ; i++){
        argv_string.at(i) = argv[i];
    }
    
    int pos = 4;// for itering through the database query result
    for( int i = 0 ; i < weather_data_3days.size ; i++){
        weather_data_3days.Wx.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_3days.size ; i++){
        weather_data_3days.PoP12h.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_3days.size ; i++){
        weather_data_3days.MinT.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_3days.size ; i++){
        weather_data_3days.MaxT.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_3days.size ; i++){
        weather_data_3days.startTime.push_back(std::string(argv[pos]));
        pos++;
    }
    for( int i = 0 ; i < weather_data_3days.size ; i++){
        weather_data_3days.endTime.push_back(std::string(argv[pos]));
        pos++;
    }

    city->City_Township_Weather_3_Days = weather_data_3days;
    return 0;
}

int Database::callback_empty(void* NotUsed, int argc, char** argv, char** azColName) {
    return 0;
}

size_t Database::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}