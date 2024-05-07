#include "database.h"

Database::Database(const char* name){
    filename = name;
    int rc = sqlite3_open(filename, &db);
    if(rc){
        std::cout<<"Can't open database: "<<sqlite3_errmsg(db)<<'\n';
    }
    else{
        std::cout<<"Opened database successfully\n";
    }
}

Database::~Database(){
    sqlite3_close(db);
}

std::pair<int,int> Database::get_direction_and_location(){    
    char* zErrMsg = 0;
    int rc;
    std::string r_command = "SELECT ctrl, location_ctrl FROM Controls";
    std::string w_command = "UPDATE Controls set ctrl = 0 ";
    std::pair<int,int>* output;
    output = new std::pair<int,int>;
    output->first = 0;
    output->second = 100;
    const char* read_command = r_command.c_str();
    const char* write_command = w_command.c_str();

    rc = sqlite3_exec(db, read_command, callback_v1, output, &zErrMsg);
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    if(output->first != 0){
        rc = sqlite3_exec(db, write_command, callback_v2, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }

    return *output;
}

City Database::get_city(int code){
    char* zErrMsg = 0;
    int rc;
    std::string command = "SELECT * FROM Location_Names WHERE code = " + std::to_string(code);
    City output;
    const char* read_command = command.c_str();

    rc = sqlite3_exec(db, read_command, callback_v3, &output, &zErrMsg);
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    return output;
}

int Database::callback_v1(void* data, int argc, char** argv, char** azColName) {
    std::pair<int,int>* result = (std::pair<int,int>*)data;
    if(argc != 2){
        std::cout<<"Error: callback function received wrong number of arguments\n";
        return 1;
    }
    result->first = atoi(argv[0]);
    result->second = atoi(argv[1]);
    return 0;
}

int Database::callback_v2(void* data, int argc, char** argv, char** azColName) {
    return 0;
}

int Database::callback_v3(void* data, int argc, char** argv, char** azColName) {
    City* result = (City*)data;
    if(argc != 5){
        std::cout<<"Error: callback function received wrong number of arguments\n";
        return 1;
    }
    result->code = atoi(argv[0]);
    std::string Chinese_name = argv[1];
    std::string English_name = argv[2];

    result->Weather_Forecast_Code_2_Days = argv[3];
    result->Weather_Forecast_Code_7_Days = argv[4];

    result->Chinese_name_city = Chinese_name.substr(0,9);
    result->Chinese_name_township = Chinese_name.substr(9);
    std::cout<<Chinese_name<<' '<<result->Chinese_name_city<<' '<<result->Chinese_name_township<<std::endl;

    result->English_name_city = English_name.substr(English_name.find_first_of(',')+2);
    result->English_name_township = English_name.substr(0, English_name.find_first_of(','));

    return 0;
}