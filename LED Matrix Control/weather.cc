#include "weather.h"

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

WeatherData_days_per_12hr get_city_township_weather_data(City &city){
    WeatherData_days_per_12hr weather_data;
    weather_data.size = 6;

    nlohmann::json weather_json_data;
    std::string readBuffer;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string url = "https://opendata.cwa.gov.tw/api/v1/rest/datastore/F-D0047-" + city.Weather_Forecast_Code_7_Days;
    std::string param1 = "Authorization=your authorization code here";
    std::string param2 = "format=JSON";
    std::string param3 = "locationName=" + city.Chinese_name_township;
    std::string param4 = "elementName=MaxT,MinT,T,PoP12h,Wx";
    std::string total_url = url + "?" + param1 + "&" + param2 + "&" + param3 + "&" + param4;
    
    std::cout<<total_url<<std::endl;

    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, total_url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if(res != CURLE_OK){std::cerr<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;}
      else{
        weather_json_data = nlohmann::json::parse(readBuffer) ;

        //std::cout<<"in place \n";

        for(int i = 0 ; i < weather_data.size ; i++){
            weather_data.MaxT.push_back(weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][4]["time"][i]["elementValue"][0]["value"].get<std::string>());
            weather_data.MinT.push_back(weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][3]["time"][i]["elementValue"][0]["value"].get<std::string>());
            weather_data.T.push_back(weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][1]["time"][i]["elementValue"][0]["value"].get<std::string>());
            weather_data.PoP12h.push_back(weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][i]["elementValue"][0]["value"].get<std::string>());
            weather_data.Wx.push_back(weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][i]["elementValue"][0]["value"].get<std::string>());
            
            
            weather_data.startTime.push_back(weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][i]["startTime"].get<std::string>());
            weather_data.Time.push_back(weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][i]["startTime"].get<std::string>());
            weather_data.endTime.push_back(weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][i]["endTime"].get<std::string>());

        }

        /*
        std::string PoP12_1 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][0]["elementValue"][0]["value"].get<std::string>();
        std::string PoP12_2 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][1]["elementValue"][0]["value"].get<std::string>();
        std::string PoP12_3 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][0]["time"][2]["elementValue"][0]["value"].get<std::string>();

        //std::cout<<"PoP12_1: "<<PoP12_1<<std::endl;

        std::string ATemp_1 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][1]["time"][0]["elementValue"][0]["value"].get<std::string>();
        std::string ATemp_2 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][1]["time"][1]["elementValue"][0]["value"].get<std::string>();
        std::string ATemp_3 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][1]["time"][2]["elementValue"][0]["value"].get<std::string>();

        //std::cout<<"ATemp_1: "<<ATemp_1<<std::endl;

        std::string Temp_1 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][0]["elementValue"][0]["value"].get<std::string>();
        std::string Temp_2 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][1]["elementValue"][0]["value"].get<std::string>();
        std::string Temp_3 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][2]["elementValue"][0]["value"].get<std::string>();

        //std::cout<<"Temp_1: "<<Temp_1<<std::endl;

        std::string Time_1 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][0]["dataTime"].get<std::string>();
        std::string Time_2 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][1]["dataTime"].get<std::string>();
        std::string Time_3 = weather_json_data["records"]["locations"][0]["location"][0]["weatherElement"][2]["time"][2]["dataTime"].get<std::string>();

        //std::cout<<"Time_1: "<<Time_1<<std::endl;

        Time_1 = Time_1.substr(5,2) + "/" + Time_1.substr(8,2) + " " + Time_1.substr(11,2) + "h";
        Time_2 = Time_2.substr(5,2) + "/" + Time_2.substr(8,2) + " " + Time_2.substr(11,2) + "h";
        Time_3 = Time_3.substr(5,2) + "/" + Time_3.substr(8,2) + " " + Time_3.substr(11,2) + "h";

        output_line1 = city.English_name_township + ',' +city.English_name_city;
        output_line2 = Time_1 + " " + Temp_1 + "C " + PoP12_1 + "%";
        output_line3 = Time_2 + " " + Temp_2 + "C " + PoP12_2 + "%";
        output_line4 = Time_3 + " " + Temp_3 + "C " + PoP12_3 + "%";

        std::cout<<output_line1<<std::endl;
        std::cout<<output_line2<<std::endl;
        std::cout<<output_line3<<std::endl;
        std::cout<<output_line4<<std::endl;

      */  
      }
      
    }
    return weather_data;
}


WeatherData_days_per_12hr get_city_weather_data(City &city){
    WeatherData_days_per_12hr weather_data;
    weather_data.size = 3;
    
    nlohmann::json weather_json_data;
    std::string readBuffer;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string url = "https://opendata.cwa.gov.tw/api/v1/rest/datastore/F-C0032-001";
    std::string param1 = "Authorization=your authorization code here";
    std::string param2 = "format=JSON";
    std::string param3 = "locationName=" + city.Chinese_name_city;
    std::cout<<param3<<std::endl;
    std::string total_url = url + "?" + param1 + "&" + param2 + "&" + param3;
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, total_url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if(res != CURLE_OK){std::cerr<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;}
      else{

        weather_json_data = nlohmann::json::parse(readBuffer) ;

        for(int i = 0 ; i < weather_data.size ; i++){
            weather_data.MaxT.push_back( weather_json_data["records"]["location"][0]["weatherElement"][4]["time"][i]["parameter"]["parameterName"].get<std::string>() ) ;
            weather_data.MinT.push_back( weather_json_data["records"]["location"][0]["weatherElement"][2]["time"][i]["parameter"]["parameterName"].get<std::string>() );      
            weather_data.PoP12h.push_back( weather_json_data["records"]["location"][0]["weatherElement"][1]["time"][i]["parameter"]["parameterName"].get<std::string>() );            
        }
      }
    }

    return weather_data;
}