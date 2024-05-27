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
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

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
const char* python_fifo_filename = "../Python Flask Server/python_fifo";
const char* api_key_filename = "../api_key/api_key.txt";
const char* api_key;
const int port_number = 15000;

enum SERVER_FLAG{
    NO_DATA = 0,
    CHANGED_page_increment = 1,
    CHANGED_page_activation = 2,
    CHANGED_location_ctrl = 3
};

// if data has changed, thi field would be not NO_DATA
std::atomic<int> server_flag(NO_DATA); 
std::atomic<int> page_increment(0);
std::atomic<int> page_activation(0);
std::atomic<int> location_ctrl(0);

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

int server_thread_function(){
    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Bind the socket to an address and port
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port_number); // Port number
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error binding socket\n";
        close(server_socket);
        return 1;
    }

    // Listen for connections
    // std::cout << "Listening for connections...\n";
    if (listen(server_socket, 5) == -1) {
        std::cerr << "Error listening on socket\n";
        close(server_socket);
        return 1;
    }

    while(true){
        // Accept a connection
        // std::cout << "Accepting connection...\n";
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            std::cerr << "Error accepting connection\n";
            close(server_socket);
            return 1;
        }

        // Receive data
        // std::cout << "Receiving data...\n";
        char buffer[1024];
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received == -1) {
            std::cerr << "Error receiving data\n";
            close(client_socket);
            close(server_socket);
            return 1;
        }

        std::string data = std::string(buffer, bytes_received);
        std::string terminate = "end";
        if(data.find(terminate) != std::string::npos){
            std::cout<<"Terminating server\n";
            close(client_socket);
            close(server_socket);
            return 0;
        }
        else{
            // std::cout<<"Data received: "<<data<<std::endl;
            switch(data[0]){
                case '0':
                    page_increment = 1;
                    server_flag = CHANGED_page_increment;
                    break;
                case '1':
                    page_increment = -1;
                    server_flag = CHANGED_page_increment;
                    break;
                case '2':
                    break;
                case '3':
                    page_activation = std::stoi(data.substr(2));
                    server_flag = CHANGED_page_activation;
                    break;
                case '4':
                    location_ctrl = std::stoi(data.substr(2));
                    server_flag = CHANGED_location_ctrl;
                    break;
                default:
                    std::cerr<<"Invalid data received\n";
                    break;
            }
        }
            
    }
}

int terminate_server_function(){
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Failed to create socket. Errno: " << errno << std::endl;
        return -1;
    }

    // Server address
    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);  // Server port number
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        std::cerr << "Error converting address. Errno: " << errno << std::endl;
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Failed to connect to server. Errno: " << errno << std::endl;
        return -1;
    }

    // Send data
    std::string data_to_send = "end";
    ssize_t bytes_sent = send(sock, data_to_send.c_str(), data_to_send.size(), 0);
    if (bytes_sent == -1) {
        std::cerr << "Failed to send data. Errno: " << errno << std::endl;
        close(sock);
        return -1;
    }

    // Close socket
    close(sock);
    return 0;

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
    std::thread server_thread(server_thread_function);// initiate server thread

    Database database(database_filename);
    Control control = database.get_control_parameters();
    page_type page = int_to_page_type(control.page_ctrl);
    City city = database.get_city(control.location_ctrl);
    std::cout<<control.page_ctrl<<' '<<control.page_activation<<' '<<control.location_ctrl<<std::endl;
    

    //test
    std::thread api_thread(api_thread_function, city, api_key);
    std::cout<<"API thread started\n";
    
    api_thread.join();
    std::cout<<"API thread joined\n";

    std::cout<<"about to update city from database\n";
    database.update_city_from_database(city);   
    std::cout<<"city updated from database\n";

    int n = 0;
    int last_city_id = -1;
    while(n<30){
        if(last_city_id != city.id){
            std::cout<<(double)(SHT30_temperature_x10)/10<<' '<<(double)(SHT30_humidity_x10)/10<<' '<<BH1750_lux<<std::endl;
            std::cout<<city.Chinese_name_city<<' '<<city.Chinese_name_township<<std::endl;
            std::cout<<city.English_name_city<<' '<<city.English_name_township<<std::endl;
            std::cout<<"高溫:"<<city.City_Township_Weather_3_Days.MaxT.at(0)<<"C ";
            std::cout<<"低溫:"<<city.City_Township_Weather_3_Days.MinT.at(0)<<"C ";
            std::cout<<"降雨機率:"<<city.City_Weather_36hr.PoP12h.at(0)<<'%'<<std::endl;
            // std::cout<<city.City_Weather_36hr.Time_Of_Last_Update_In_Seconds<<std::endl;
            last_city_id = city.id;
        }

        switch(server_flag){
            case CHANGED_page_increment:
                page = int_to_page_type((int)page + page_increment);
                if(page_increment==1){std::cout<<"Next Page\n";}
                else if(page_increment==-1){std::cout<<"Last Page\n";}
                page_increment = 0;
                server_flag = NO_DATA;
                break;
            case CHANGED_page_activation:
                server_flag = NO_DATA;
                // page_activation = 0;
                break;
            case CHANGED_location_ctrl:
                city = database.get_city(location_ctrl);
                api_thread = std::thread(api_thread_function, city, api_key);
                server_flag = NO_DATA;
                break;
            default:
                break;
        }

        usleep(1000000);
        if(api_thread.joinable()){
            api_thread.join();
            // std::cout<<"API thread joined\n";
            database.update_city_from_database(city);
        }        
        
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
    terminate_server_function();
    server_thread.join();
    close_chip();
    
}