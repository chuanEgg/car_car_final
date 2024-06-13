#include <iostream>
#include <stdio.h>

#include "include/led-matrix.h"
#include "include/graphics.h"
#include "modules.h"
#include "database.h"
#include "led-control.h"

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

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>


volatile bool interrupt_received = false;
struct gpiod_chip *chip = nullptr;


const int welcome_audio_num = 3;
const char* welcome_audio_filenames[3] = {"../audio/Welcome Home Amagami.wav", "../audio/Welcome Home Nisekoi.wav", "../audio/Welcome Home One Room.wav"};
const int goodbye_audio_num = 3;
const char* goodbye_audio_filenames[1] = {"../audio/Goodbye Onimai.wav"};



const char* filenames[1] = {"../images/sakura.gif"};
const char* time_font_filename = "../fonts/5x7.bdf";
const char* database_filename = "../../SQLite Database/database.db";
const char* python_fifo_filename = "../Python Flask Server/python_fifo";
const char* api_key_filename = "../api_key/api_key.txt";
const char* api_key;
const int port_number = 15000;

SDL_AudioDeviceID audio_device;

enum SERVER_FLAG{
    NO_DATA = 0,
    CHANGED_page_increment,
    CHANGED_page_activation,
    CHANGED_location_ctrl,
    CHANGED_MOTOR_PWM,
    CHANGED_MOTOR_TIME_DURATION,
    CHANGED_FAN_PWM,
    CHANGED_FAN_TIME_DURATION
};

// if data has changed, thi field would be not NO_DATA
std::atomic<int> server_flag(NO_DATA); 
std::atomic<int> page_increment(0);
std::atomic<int> page_activation(0);
std::atomic<int> location_ctrl(0);
std::atomic<int> motor_pwm(1);
std::atomic<int> motor_time_duration(30);
std::atomic<int> fan_pwm(1);
std::atomic<int> fan_time_duration(30);

std::atomic<int> SHT30_temperature_x10(0), SHT30_humidity_x10(0), BH1750_lux(0);

// sensor values on connected to arduino
std::vector<int> distance_sensor_values;
int hall_sensor_value = 0;


SHT30* sht30;
BH1750* bh1750;
Arduino_Peripherals* arduino_peripherals;


void signal_handler(int signal){
    interrupt_received = true;
}

void api_thread_function(const City& city, const char* api_key){
    Database database(database_filename);
    if( database.update_database_from_API(city, api_key) ){
        std::cerr<<"Failed to update database from API"<<std::endl;
    }
}

void initiate_peripherals(){
    sht30 = new SHT30(GND);
    bh1750 = new BH1750(GND);
    bh1750->change_mode(ONE_TIME_L_RESOLUTION_MODE);
    arduino_peripherals = new Arduino_Peripherals(0x03);

    std::vector<double> temp_humidity;
    usleep(100000);
    temp_humidity = sht30->get_temperature_humidity();
    SHT30_temperature_x10 = (int)(round(temp_humidity.at(0)*10));
    SHT30_humidity_x10 = (int)(round(temp_humidity.at(1)*10));
    BH1750_lux = bh1750->get_lux();
    usleep(100000);
    arduino_peripherals->motor_pwm_control(0,150);
    arduino_peripherals->motor_pwm_control(1,255);
}

void update_sensors(){
    std::vector<double> temp_humidity;
    
    temp_humidity = sht30->get_temperature_humidity();
    SHT30_temperature_x10 = (int)(round(temp_humidity.at(0)*10));
    SHT30_humidity_x10 = (int)(round(temp_humidity.at(1)*10));
    BH1750_lux = bh1750->get_lux();

    distance_sensor_values = arduino_peripherals->get_all_distance_sensor_value(3);
    // distance_sensor_values =  arduino_peripherals->get_all_distance_sensor_value_one_by_one(3);
    hall_sensor_value = arduino_peripherals->get_hall_sensor_value(0);
}


void LED_strip_show_RGB(int speed){
    static int n = 0;
    if(n<512){
        arduino_peripherals->led_control(0,abs(n%256-256),0,0);
    }
    else if(n<1024){
        arduino_peripherals->led_control(0,0,abs(n%256-256),0);
    }
    else if(n<1536){
        arduino_peripherals->led_control(0,0,0,n%256-256);
    }
    else{
        n = 0;
    }
    n+=speed;
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
            std::cout<<"Data received: "<<data<<std::endl;
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
                case '5':
                    motor_pwm = std::stoi(data.substr(2));
                    server_flag = CHANGED_MOTOR_PWM;
                    break;
                case '6':
                    motor_time_duration = std::stoi(data.substr(2));
                    server_flag = CHANGED_MOTOR_TIME_DURATION;
                    break;
                case '7':    
                    fan_pwm = std::stoi(data.substr(2));
                    server_flag = CHANGED_FAN_PWM;
                    break;
                case '8':
                    fan_time_duration = std::stoi(data.substr(2));
                    server_flag = CHANGED_FAN_TIME_DURATION;
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


void initiate_sound(){
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        std::cerr << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
        // SDL_Quit();
        // return 1;
    }

    // Open audio device
    SDL_AudioSpec desired, obtained;
    desired.freq = 48000;
    desired.format = MIX_DEFAULT_FORMAT;
    desired.channels = 2;
    desired.samples = 4096;
    desired.callback = NULL;  // NULL for using SDL's audio callback
    const char* device_name = SDL_GetAudioDeviceName(3, 0); // choose device index from available devices
    audio_device = SDL_OpenAudioDevice(device_name, 0, &desired, &obtained, 0);
    
    
    if (audio_device == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
    } 
    else {
        SDL_Log("Audio device opened successfully: %s", device_name);
    }
}

void play_welcome_audio(){
    int audio_index = rand()%welcome_audio_num;
    std::cout<<"in play welcome audio\n";
    Mix_Chunk *sound = Mix_LoadWAV(welcome_audio_filenames[audio_index]);
    if (sound == NULL) {
        SDL_Log("Failed to load sound: %s", Mix_GetError());
    } 
    else if( Mix_PlayChannel(-1, sound, 0) != 0){
        std::cerr<<"Failed to play sound: "<<Mix_GetError()<<std::endl;
    }

    while (Mix_Playing(-1)) {
        SDL_Delay(100);
    }
    Mix_FreeChunk(sound);
    std::cout<<"out play welcome audio\n";
}

void play_goodbye_audio(){
    int audio_index = rand()%goodbye_audio_num;
    Mix_Chunk *sound = Mix_LoadWAV(goodbye_audio_filenames[audio_index]);
    if (sound == NULL) {
        SDL_Log("Failed to load sound: %s", Mix_GetError());
    } 
    else if( Mix_PlayChannel(-1, sound, 0) != 0){
        std::cerr<<"Failed to play sound: "<<Mix_GetError()<<std::endl;
    }

    while (Mix_Playing(-1)) {
        SDL_Delay(100);
    }
    Mix_FreeChunk(sound);
}

void play_rain_audio(){
    Mix_Chunk *sound = Mix_LoadWAV("../audio/Rain.wav");
    if (sound == NULL) {
        SDL_Log("Failed to load sound: %s", Mix_GetError());
    } 
    else if( Mix_PlayChannel(-1, sound, 0) != 0){
        std::cerr<<"Failed to play sound: "<<Mix_GetError()<<std::endl;
    }

    while (Mix_Playing(-1)) {
        SDL_Delay(100);
    }
    Mix_FreeChunk(sound);
}

void close_sound(){
    Mix_CloseAudio();
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
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
    
    initiate_sound();

    initiate_peripherals();// initiate sensors

    
    usleep(5000000);// sleep to prevent conflict of initiating LED matrix and I2C devices

    LED_Matrix led_matrix(time_font_filename);// initiate LED matrix


    std::thread server_thread(server_thread_function);// initiate server thread
    
    Database database(database_filename);
    Control control = database.get_control_parameters();
    Page page = int_to_page(control.page_ctrl);
    City city = database.get_city(control.location_ctrl);

    std::thread api_thread(api_thread_function, city, api_key);// update the weather data in the database
    if(api_thread.joinable()){
        api_thread.join();
    }

    if( database.update_city_from_database(city) == 1 ){
        std::cerr<<"Failed to update city from database"<<std::endl;
        return 1;
    }

    led_matrix.change_page(page, SHT30_temperature_x10, SHT30_humidity_x10, BH1750_lux, city);// initialize the LED matrix content

    int n = 0;
    int last_city_id = -1;

    int distance_threashold = 10;

    bool page_stopped = 0;

    // for testing
    std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
    // for testing
    while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time).count() < 60){
        
        BH1750_lux = bh1750->get_lux();
        
        if(BH1750_lux < 100 ){
            led_matrix.change_page(Page::Stop, SHT30_temperature_x10, SHT30_humidity_x10, BH1750_lux, city);
            page_stopped = 1;
            usleep(500000);
            continue;
        }

        if(page_stopped == 1){
            led_matrix.change_page(page, SHT30_temperature_x10, SHT30_humidity_x10, BH1750_lux, city);
            page_stopped = 0;
        }
        

        distance_sensor_values = arduino_peripherals->get_all_distance_sensor_value(3);
        //sensor 0 face out, sensor 1 face in, sensor 2 face unbrella
        if(distance_sensor_values[1] < distance_threashold &&  distance_sensor_values[1] != 0){
            // todo 
            // play goodbye music
            arduino_peripherals->led_control(0,0,255,0);
            play_goodbye_audio();

            if(std::stoi(city.City_Township_Weather_3_Days.PoP12h.at(0)) > distance_threashold){
                // todo
                // play rain music
                arduino_peripherals->led_control(0,255,255,255);
                
                
                
            }
        }
        else if(distance_sensor_values[0] < distance_threashold &&  distance_sensor_values[0] != 0){
            // todo
            // play welcome home music   
            arduino_peripherals->led_control(0,0,255,0);
            play_welcome_audio();
        }

        else{
            // arduino_peripherals->led_control(0,0,0,255);
        }
        
        // LED_strip_show_RGB(15); 
        // arduino_peripherals->led_control(0,255,255,255);

        // std::cout<<(double)(SHT30_temperature_x10)/10<<' '<<(double)(SHT30_humidity_x10)/10<<' '<<BH1750_lux<<std::endl;

        if(last_city_id != city.id){
            std::cout<<"Switched to :";
            std::cout<<city.Chinese_name_city<<' '<<city.Chinese_name_township<<std::endl;
            std::cout<<city.English_name_city<<' '<<city.English_name_township<<std::endl;
            last_city_id = city.id;
        }

        
        switch(server_flag){
            case CHANGED_page_increment:
                page = int_to_page((int)page + page_increment);
                if(page_increment >= 1){
                    led_matrix.change_page(page, SHT30_temperature_x10, SHT30_humidity_x10, BH1750_lux, city);
                }
                else if(page_increment <= -1){
                    led_matrix.change_page(page, SHT30_temperature_x10, SHT30_humidity_x10, BH1750_lux, city);
                }
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
            case CHANGED_MOTOR_PWM:
                arduino_peripherals->motor_pwm_control(0, motor_pwm);
                server_flag = NO_DATA;
                break;
            case CHANGED_MOTOR_TIME_DURATION:  
                arduino_peripherals->motor_time_control(0, motor_time_duration);
                server_flag = NO_DATA;
                break;
            case CHANGED_FAN_PWM:
                arduino_peripherals->motor_pwm_control(1, fan_pwm);
                server_flag = NO_DATA;
                break;
            case CHANGED_FAN_TIME_DURATION:
                arduino_peripherals->motor_time_control(1, fan_time_duration);
                server_flag = NO_DATA;
                break;
            default:
                break;
        }
        

        // usleep(10000);
        if(api_thread.joinable()){
            api_thread.join();
            // std::cout<<"API thread joined\n";
            database.update_city_from_database(city);
            led_matrix.change_page(page, SHT30_temperature_x10, SHT30_humidity_x10, BH1750_lux, city);
        }

        
        n++;
        update_sensors();
        std::cout<<distance_sensor_values.at(0)<<' '<<distance_sensor_values.at(1)<<' '<<distance_sensor_values.at(2)<<'?';
        std::cout<<BH1750_lux<<' '<<hall_sensor_value<<std::endl;
        // arduino_peripherals->led_control(0,n%256,n%256,n%256);
        // arduino_peripherals->motor_pwm_control(0,100);

    }

    led_matrix.change_page(Page::Stop, SHT30_temperature_x10, SHT30_humidity_x10, BH1750_lux, city);
    arduino_peripherals->led_control(0,0,0,0);
    arduino_peripherals->motor_pwm_control(0,1);
    terminate_server_function();
    server_thread.join();

    close_sound();

    return 0;
}