#include "modules.h"


// int initialize_chip(){
//     chip = gpiod_chip_open_by_name(chipname);
//     if (!chip) {
//         std::cerr << "Failed to open " << chipname << std::endl;
//         return 1;
//     }
//     return 0;
// }

// int close_chip(){
//     gpiod_chip_close(chip);
//     return 0;
// }

// EC11E_Mode& operator++(EC11E_Mode& mode){
//     switch(mode){
//         case NO_ROTATION:
//             mode = NO_ROTATION_SWITCH_PRESSED;
//             break;
//         case NO_ROTATION_SWITCH_PRESSED:
//             mode = CLOCKWISE;
//             break;
//         case CLOCKWISE:
//             mode = CLOCKWISE_SWITCH_PRESSED;
//             break;
//         case CLOCKWISE_SWITCH_PRESSED:
//             mode = COUNTER_CLOCKWISE;
//             break;
//         case COUNTER_CLOCKWISE:
//             mode = COUNTER_CLOCKWISE_SWITCH_PRESSED;
//             break;
//         case COUNTER_CLOCKWISE_SWITCH_PRESSED:
//             mode = NO_ROTATION;
//             break;
//         default:
//             std::cerr<<"EC11E error at incrementing mode"<<std::endl;
//             break;
//     }
//     return mode;
// }

// EC11E_Mode operator++(EC11E_Mode& mode, int num){
//     for(int i = 0 ; i < num ; i++){
//         ++mode;
//     }
//     return mode;
// }


// EC11E::EC11E(int pinA, int pinB, int pin_Switch){
//     bool ok = true;
//     this->pinA = pinA;
//     this->pinB = pinB;
//     this->pin_Switch = pin_Switch;

//     timeout.tv_sec = 0;
//     timeout.tv_nsec = 0;   

//     if( !( pinA == 5 || pinA == 6 || pinA == 12 || pinA == 13 || pinA == 16 || pinA == 17 || pinA == 18 || pinA == 19 || pinA == 20 || pinA == 21 || pinA == 22 || pinA == 23 || pinA == 24 || pinA == 25 || pinA == 26 || pinA == 27 ) ){
//         std::cerr<<"EC11E error at pinA in pin assignment"<<std::endl;
//         ok = false;
//     }
//     if( !( pinB == 5 || pinB == 6 || pinB == 12 || pinB == 13 || pinB == 16 || pinB == 17 || pinB == 18 || pinB == 19 || pinB == 20 || pinB == 21 || pinB == 22 || pinB == 23 || pinB == 24 || pinB == 25 || pinB == 26 || pinB == 27 ) ){
//         std::cerr<<"EC11E error at pinB in pin assignment"<<std::endl;
//         ok = false;
//     }
//     if( !( pin_Switch == 5 || pin_Switch == 6 || pin_Switch == 12 || pin_Switch == 13 || pin_Switch == 16 || pin_Switch == 17 || pin_Switch == 18 || pin_Switch == 19 || pin_Switch == 20 || pin_Switch == 21 || pin_Switch == 22 || pin_Switch == 23 || pin_Switch == 24 || pin_Switch == 25 || pin_Switch == 26 || pin_Switch == 27 ) ){
//         std::cerr<<"EC11E error at pin_Switch in pin assignment"<<std::endl;
//         ok = false;
//     }
//     if(pinA == pinB || pinA == pin_Switch || pinB == pin_Switch){
//         std::cerr<<"EC11E error at pin assignment, repeated pin assignment "<<std::endl;
//         ok = false;
//     }

//     if(ok){
//         //Open GPIO lines
//         A = gpiod_chip_get_line(chip, pinA);
//         B = gpiod_chip_get_line(chip, pinB);
//         Button = gpiod_chip_get_line(chip, pin_Switch);

//         //Request line event for rising and falling edge
//         int ret_A = gpiod_line_request_falling_edge_events(A, "EC11E Rotary Encoder");
//         int ret_B = gpiod_line_request_falling_edge_events(B, "EC11E Rotary Encoder");
//         //int ret_Button = gpiod_line_request_both_edge_events(Button, "EC11E Rotary Encoder");
//         int ret_Button = gpiod_line_request_input(Button, "EC11E Rotary Encoder");

//         //Check if request is successful
//         if(ret_A < 0){
//             std::cerr<<"EC11E error at requesting line event for pinA"<<std::endl;
//         }
//         if(ret_B < 0){
//             std::cerr<<"EC11E error at requesting line event for pinB"<<std::endl;
//         }
//         if(ret_Button < 0){
//             std::cerr<<"EC11E error at requesting line event for pin_Switch"<<std::endl;
//         }  
//     }
// }

// EC11E::~EC11E(){
//     gpiod_line_release(A);
//     gpiod_line_release(B);
//     gpiod_line_release(Button);
// }

// EC11E_Mode EC11E::get_rotation(){
//     //int A_value = gpiod_line_get_value(A);
//     //int B_value = gpiod_line_get_value(B);
//     //int Button_value = gpiod_line_get_value(Button);

//     EC11E_Mode mode = NO_ROTATION;
//     int ret_A = gpiod_line_event_wait(A, &timeout);
//     int ret_B = gpiod_line_event_wait(B, &timeout);
//     //int ret_Button = gpiod_line_event_wait(Button, &timeout);

//     if(ret_A < 0){
//         std::cerr<<"EC11E error at waiting for line event for pinA"<<std::endl;
//         return NO_ROTATION;
//     }
//     if (ret_B < 0){
//         std::cerr<<"EC11E error at waiting for line event for pinB"<<std::endl;
//         return NO_ROTATION;
//     }
//     /*if(ret_Button < 0){
//         std::cerr<<"EC11E error at waiting for line event for pin_Switch"<<std::endl;
//         return NO_ROTATION;
//     }*/

//     if(ret_A > 0 && ret_B > 0){
//         if( gpiod_line_event_read(A, &event_A) == 0  && gpiod_line_event_read(B, &event_B) == 0){
//             if(event_A.ts.tv_sec > event_B.ts.tv_sec){
//                 mode = CLOCKWISE;
//             }
//             else if(event_A.ts.tv_sec == event_B.ts.tv_sec){
//                 if(event_A.ts.tv_nsec > event_B.ts.tv_nsec){
//                     mode = CLOCKWISE;
//                 }
//                 else{
//                     mode = COUNTER_CLOCKWISE;
//                 }
//             }
//             else{
//                 mode = COUNTER_CLOCKWISE;
//             }
//         }
//         else{
//             std::cerr<<"EC11E error at reading line event for pinA or pinB"<<std::endl;
//         }
//     }
//     else if(ret_A > 0 && ret_B == 0){
//         if( gpiod_line_event_read(A, &event_A) != 0 ){
//             std::cerr<<"EC11E error at reading line event for pinA or pinB"<<std::endl;
//         }
//         mode = CLOCKWISE;
//     }
//     else if(ret_A == 0 && ret_B > 0){
//         if( gpiod_line_event_read(B, &event_B) != 0 ){
//             std::cerr<<"EC11E error at reading line event for pinA or pinB"<<std::endl;
//         }
//         mode = COUNTER_CLOCKWISE;
//     }

//     //Check if button is pressed
//     if(gpiod_line_get_value(Button)){
//         ++mode;
//     }

//     std::cerr<<ret_A<<" "<<ret_B<<" "<<mode<<std::endl;


    


//     /*
//     if(Last_A_value != A_value){
//         if(A_value != B_value){
//             if(Button_value == 0){
//                 mode = CLOCKWISE;
//             }
//             else{
//                 mode = CLOCKWISE_SWITCH_PRESSED;
//             }
//         }
//         else{
//             if(Button_value == 0){
//                 mode = COUNTER_CLOCKWISE;
//             }
//             else{
//                 mode = COUNTER_CLOCKWISE_SWITCH_PRESSED;
//             }
//         }
//         Last_A_value = A_value;
//     }
//     else{
//         if(Button_value == 0){
//             mode = NO_ROTATION;
//         }
//         else{
//             mode = NO_ROTATION_SWITCH_PRESSED;
//         }
//     }
//     */
//     return mode;
// }


BH1750::BH1750(ADDRESS_SELECT type){
    current_mode = POWER_DOWN;

    switch(type){
        case GND:
            device_address = 0x23;
            break;
        case VCC:
            device_address = 0x5C;
            break;
        default:
            std::cerr<<"BH1750 error at determining I2C device address, Address select error"<<std::endl;
            break;
    }

    file = open(filename, O_RDWR);
    if (file < 0) {
        std::cerr<<"BH1750 error at opening file"<<std::endl;
    }
    else if (ioctl(file, I2C_SLAVE, device_address) < 0) {
        std::cerr<<"BH1750 error at initializing"<<std::endl;
    }

    usleep(1);

    //little endian
    //power on command
    uint8_t buf_send[1];
    buf_send[0] = 0b00000001;
    if (write(file, buf_send, 1) != 1) {
        // If I2C power on command failed to deliver
        std::cerr<<"BH1750 error at power on"<<std::endl;
    }
    else{
        // Successfully powered on
        current_mode = POWER_ON;
    }
    
}

BH1750::~BH1750(){

    switch (current_mode) {
        case POWER_DOWN:
            break;
        case POWER_ON:
            break;
        case RESET:
            break;
        case CONTINUOUSLY_H_RESOLUTION_MODE:
            break;
        case CONTINUOUSLY_H_RESOLUTION_MODE2:
            break;
        case CONTINUOUSLY_L_RESOLUTION_MODE:
            break;
        case ONE_TIME_H_RESOLUTION_MODE:
            break;
        case ONE_TIME_H_RESOLUTION_MODE2:
            break;
        case ONE_TIME_L_RESOLUTION_MODE:
            break;
        default:
            std::cerr<<"BH1750 error at closing, mode error"<<std::endl;
            break;
    }

    //little endian
    //power down command    
    uint8_t buf_send[1];
    buf_send[0] = 0b00000000;
    if (write(file, buf_send, 1) != 1) {
    // If I2C power down command failed to deliver
        std::cerr<<"BH1750 error at power down"<<std::endl;
    }
    close(file);
}

//return positive int for lux on success, -1 for failure
//blocking for 180ms or 24 ms if using one of the three one time measurement modes
int BH1750::get_lux(){
    useconds_t wait_time = 0;
    switch(current_mode){
        case ONE_TIME_H_RESOLUTION_MODE:
            wait_time = 180000;//wait for at least 180ms(max measurment time), specified on datasheet page 5

        case ONE_TIME_H_RESOLUTION_MODE2:
            wait_time = 180000;//wait for at least 180ms(max measurment time), specified on datasheet page 5

        case ONE_TIME_L_RESOLUTION_MODE:
            wait_time = 24000;//wait for at least 24ms(max measurment time), specified on datasheet page 5

            uint8_t buf_send[1];
            buf_send[0] = 0b00000001;
            if (write(file, buf_send, 1) != 1) {
                std::cerr<<"BH1750 error at powering on for one time measurement : "<<std::bitset<8>(current_mode)<<std::endl;
                return -1;
            }

            usleep(1);
            buf_send[0] = current_mode;
            if (write(file, buf_send, 1) != 1) {
                std::cerr<<"BH1750 error at sending measurement command for one time measurement : "<<std::bitset<8>(current_mode)<<std::endl;
                return -1;
            }

            usleep(wait_time);//wait for measurement to complete

            break;
        case CONTINUOUSLY_H_RESOLUTION_MODE:
        case CONTINUOUSLY_H_RESOLUTION_MODE2:
        case CONTINUOUSLY_L_RESOLUTION_MODE:
            break;

        default:
            std::cerr<<"BH1750 error at getting lux, mode error"<<std::endl;
            break;
    }

    uint8_t buf_recived[2];
    if (read(file, buf_recived, 2) != 2) {
        //If I2C failed to read lux
        std::cerr<<"BH1750 error at reading lux"<<std::endl;
        return -1;
    } 
    else {
        int lux = ( int )( (double)(buf_recived[0] << 8 | buf_recived[1]) / 1.2 );
        return lux;
    }
}

int BH1750::change_mode(BH1750_Mode mode){
    switch(current_mode){
        case CONTINUOUSLY_H_RESOLUTION_MODE:
        case CONTINUOUSLY_H_RESOLUTION_MODE2:
        case CONTINUOUSLY_L_RESOLUTION_MODE:
            uint8_t buf_send[1];
            buf_send[0] = mode;
            if (write(file, buf_send, 1) != 1) {
                //If I2C failed to change mode
                std::cerr<<"BH1750 error at changing to mode : "<<std::bitset<8>(mode)<<std::endl;
                return -1;
            }
            else{
                current_mode = mode;
                return 1;
            }
        case POWER_ON:
        case ONE_TIME_H_RESOLUTION_MODE: //because these three modes already return to power down after measuring once
        case ONE_TIME_H_RESOLUTION_MODE2:
        case ONE_TIME_L_RESOLUTION_MODE:
            current_mode = mode;
            return 1;
        default:
            std::cerr<<"BH1750 error at changing mode, mode error"<<std::endl;
            return -1;
    }

}

MMA8452::MMA8452(ADDRESS_SELECT type){
    switch(type){
        case GND:
            device_address = 0x1C;
            break;
        case VCC:
            device_address = 0x1D;
            break;
        default:
            std::cerr<<"BH1750 error at determining I2C device address, Address select error"<<std::endl;
            break;
    }
    file = open(filename, O_RDWR);
    if (file < 0) {
        std::cerr<<"MMA8452 error at opening file"<<std::endl;
    }
    if (ioctl(file, I2C_SLAVE, device_address) < 0) {
        std::cerr<<"MMA8452 error at initializing"<<std::endl;
    }

    usleep(1);

    //little endian
    //power on command
    uint8_t buf_send[2];
    buf_send[0] = 0x2A;//register address
    buf_send[1] = 1;//data 
    if (write(file, buf_send, 2) != 2) {
        // I2C power on command failed to deliver
        std::cerr<<"MMA8452 error at initializing"<<std::endl;
    }
}

MMA8452::~MMA8452(){
    //little endian
    //power down command
    uint8_t buf_send[2];
    buf_send[0] = 0x2A;//register address
    buf_send[1] = 0;//data 
    if (write(file, buf_send, 2) != 2) {
        // I2C power down command failed to deliver
        std::cerr<<"MMA8452 error at powering down"<<std::endl;
    }
    close(file);
}

std::vector<int> MMA8452::get_acceleration(){
    std::vector<int> acceleration(3,INT_MIN);
    uint8_t buf_send[1];
    uint8_t buf_recived[4];
    buf_send[0] = 0x01;//register address
    if (write(file, buf_send, 1) != 1) {
        // I2C transaction failed
        std::cerr<<"MMA8452 error at reading acceleration"<<std::endl;
        return acceleration;
    }
    else if (read(file, buf_recived, 4) != 4 ) {
            // I2C transaction failed
            std::cerr<<"MMA8452 error at reading acceleration"<<std::endl;
            return acceleration;
    } 
    else {
        acceleration.at(0) = ( buf_recived[0] << 8 | buf_recived[1] ) >> 4 ;
        acceleration.at(1) = ( buf_recived[2] << 8 | buf_recived[3] ) >> 4 ;
    }

    buf_send[0] = 0x05;//register address
    if (write(file, buf_send, 1) != 1) {
        // I2C transaction failed
        std::cerr<<"MMA8452 error at reading acceleration"<<std::endl;
        return acceleration;
    }
    else if (read(file, buf_recived, 4) != 4) {
            // I2C transaction failed
            std::cerr<<"MMA8452 error at reading acceleration"<<std::endl;
            return acceleration;
    } 
    else {
        acceleration.at(2) = ( buf_recived[0] << 8 | buf_recived[1] ) >> 4 ;
        return acceleration;
    }

}

SHT30::SHT30(ADDRESS_SELECT type){
    switch(type){
        case GND:
            device_address = 0x44;
            break;
        case VCC:
            device_address = 0x45;
            break;
        default:
            std::cerr<<"SHT30 error at determining I2C device address"<<std::endl;
            break;
    }
    file = open(filename, O_RDWR);
    if (file < 0) {
        std::cerr<<"SHT30 error at opening file"<<std::endl;
    }
    if (ioctl(file, I2C_SLAVE, device_address) < 0) {
        std::cerr<<"SHT30 error at initializing"<<std::endl;
    }

    usleep(500); //wait for the sensor to initialize after power up, at datasheet p7 table4

    //little endian
    //command for setting up using 1 mps, High repeatability mode, at page 11 of spec
    uint8_t buf_send[2];
    buf_send[0] = 0x21;//MSB
    buf_send[1] = 0x30;//LSB 
    if (write(file, buf_send, 2) != 2) {
        // I2C power on command failed to deliver
        std::cerr<<"SHT30 error at initializing it's mode to High repeatability mode"<<std::endl;
    }
    last_measurement_time = std::chrono::system_clock::now();// set to prevent access of data before first measurement  
    temperature_humidity.resize(2,0);// set initial value to data
}

SHT30::~SHT30(){
    //little endian
    //command for canceling constant measuring 
    uint8_t buf_send[2];
    buf_send[0] = 0x30;//MSB
    buf_send[1] = 0x93;//LSB
    std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::system_clock::now();
    int time_diff = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_measurement_time).count();
    if(time_diff < 15){
        usleep(16-time_diff); //extra 15ms for it to complete the former measurement
    }
    if (write(file, buf_send, 2) != 2) {
        // I2C change to idel mode command failed to deliver
        std::cerr<<"SHT30 error at changing to idle mode"<<std::endl;
    }
    close(file);
}

//returns temp and humidity both 0 if uninitialized.
std::vector<double> SHT30::get_temperature_humidity(){

    std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::system_clock::now();
    if(std::chrono::duration_cast<std::chrono::seconds>(current_time - last_measurement_time).count() >= 1){ //std::chrono::system_clock::now() measures time in nanoseconds
        uint8_t buf_send[2];
        uint8_t buf_recived[6];
        buf_send[0] = 0xE0;
        buf_send[1] = 0x00;
        if (write(file, buf_send, 2) != 2) {
            std::cerr<<"SHT30 error at sending command to retrieve temperature and humidity"<<std::endl;
        }
        else if (read(file, buf_recived, 6) != 6) {
            std::cerr<<"SHT30 error at reading temperature and humidity"<<std::endl;
        } 
        else {
            temperature_humidity.at(0) = (double)( buf_recived[0] << 8 | buf_recived[1] ) * 175 / 65535 - 45;//convert the data to readable format
            temperature_humidity.at(1) = (double)( buf_recived[3] << 8 | buf_recived[4] ) * 100 / 65535;
        }
        last_measurement_time = std::chrono::system_clock::now();
    }
    return temperature_humidity;
}

// Push_Button::Push_Button(int pin){
//     this->pin = pin;
//     Button = gpiod_chip_get_line(chip, pin);
//     int ret_Button = gpiod_line_request_input(Button, "EC11E Rotary Encoder");
//     if(ret_Button < 0){
//         std::cerr<<"Push_Button error at requesting line event for pin"<<std::endl;
//     }
// }

// Push_Button::~Push_Button(){
//     gpiod_line_release(Button);
// }

// bool Push_Button::get_state(){
//     return gpiod_line_get_value(Button);
// }

Arduino_Peripherals::Arduino_Peripherals(int address){
    device_address = address;
    file = open(filename, O_RDWR);
    if (file < 0) {
        std::cerr<<"Arduino_Peripherals error at opening file"<<std::endl;
    }
    if (ioctl(file, I2C_SLAVE, device_address) < 0) {
        std::cerr<<"Arduino_Peripherals error at initializing"<<std::endl;
    }

}
Arduino_Peripherals::~Arduino_Peripherals(){
    uint8_t buf_send[1];
    buf_send[0] = 0b00000000;
    if (write(file, buf_send, 1) != 1) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at powering down"<<std::endl;
    }
    close(file);
}


int Arduino_Peripherals::led_control(int LED_number, int pwm_R, int pwm_G, int pwm_B){
    uint8_t buf_send[5];
    buf_send[0] = 0b00000001;
    buf_send[1] = LED_number;
    buf_send[2] = pwm_R;
    buf_send[3] = pwm_G;
    buf_send[4] = pwm_B;

    if (write(file, buf_send, 5) != 5) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at controlling LED"<<std::endl;
        return 1;
    }
    return 0;

}


int Arduino_Peripherals::motor_pwm_control(int motor_number, int pwm_value){
    uint8_t buf_send[4];
    buf_send[0] = 0b00000010;
    buf_send[1] = motor_number;
    buf_send[2] = 2;
    buf_send[3] = pwm_value;
    if (write(file, buf_send, 4) != 4) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at controlling motor"<<std::endl;
        return 1;
    }
    return 0;

}

int Arduino_Peripherals::motor_time_control(int motor_number, unsigned int time_duration){
    uint8_t buf_send[7];
    buf_send[0] = 0b00000010;
    buf_send[1] = motor_number;
    buf_send[2] = 1;

    buf_send[3] = (time_duration >> 24) % 256;
    buf_send[4] = (time_duration >> 16) % 256;
    buf_send[5] = (time_duration >> 8) % 256;
    buf_send[6] = time_duration % 256;
    if (write(file, buf_send, 7) != 7) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at controlling motor"<<std::endl;
        return 1;
    }
    return 0;
}


//Function to get the distance sensor value on the Arduino
//sensor_number: 0 (Sensor 1), 1 (Sensor 2), 2 (Sensor 3)
//return positive int from 0 ~ 65535 on success, -1 on failure
int Arduino_Peripherals::get_distance_sensor_value(int sensor_number){
    uint8_t buf_send[3];
    buf_send[0] = 0b00000011;
    buf_send[1] = 1;
    buf_send[2] = sensor_number;
    if (write(file, buf_send, 3) != 3) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at getting distance sensor value"<<std::endl;
        return -1;
    }

    uint8_t buf_recived[2];
    if (read(file, buf_recived, 2) != 2) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at getting distance sensor value"<<std::endl;
        return -1;
    } 
    else {
        return ( int )(buf_recived[0] << 8 | buf_recived[1]);
    }

}


//Function to get all the distance sensors' value on the Arduino
//return std::vector<int> with variable length, each element is a positive int from 0 ~ 65535 on success, -1 on failure
std::vector<int> Arduino_Peripherals::get_all_distance_sensor_value(){
    
    std::vector<int> distances(distance_sensor_num,0);
    if(distance_sensor_num == 0){
        uint8_t buf_send[2];
        buf_send[0] = 0b00000011;
        buf_send[1] = 0;
        if (write(file, buf_send, 2) != 2) {
            // I2C power on command failed to deliver
            std::cerr<<"Arduino_Peripherals error at getting the number of sensors at writing to i2c"<<std::endl;
            return distances;
        }

        uint8_t buf_recived[1];
        if (read(file, buf_recived, 1) != 1) {
            // I2C power on command failed to deliver
            std::cerr<<"Arduino_Peripherals error at getting the number of sensors at reading i2c"<<std::endl;
            return distances;
        }
        distance_sensor_num = (int)buf_recived[0];
    }


    
    uint8_t buf_send[2];
    buf_send[0] = 0b00000011;
    buf_send[1] = 2;
    if (write(file, buf_send, 2) != 2) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at sending i2c command to retrieve distance sensor value"<<std::endl;
        return distances;
    }

    uint8_t* buf_recived = new uint8_t[distance_sensor_num*2];
    if (read(file, buf_recived, distance_sensor_num*2) != distance_sensor_num*2) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at getting distance sensor value from i2c"<<std::endl;
        return distances;
    } 
    for(int i = 0 ; i < distance_sensor_num ; i++){
        distances.at(i) = ( int )( ( (int)(buf_recived[i*2]) ) << 8 | ( (int)(buf_recived[i*2+1]) ));
    }
    delete buf_recived;
    return distances;
}

std::vector<int> Arduino_Peripherals::get_all_distance_sensor_value(int num_sensors){
    distance_sensor_num = num_sensors;

    std::vector<int> distances(distance_sensor_num,0);
    
    uint8_t buf_send[2];
    buf_send[0] = 0b00000011;
    buf_send[1] = 2;
    if (write(file, buf_send, 2) != 2) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at sending i2c command to retrieve distance sensor value"<<std::endl;
        return distances;
    }

    uint8_t* buf_recived = new uint8_t[distance_sensor_num*2];
    if (read(file, buf_recived, distance_sensor_num*2) != distance_sensor_num*2) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at getting distance sensor value from i2c"<<std::endl;
        return distances;
    } 
    for(int i = 0 ; i < distance_sensor_num ; i++){
        distances.at(i) = ( int )( ( (int)(buf_recived[i*2]) ) << 8 | ( (int)(buf_recived[i*2+1]) ));
    }
    delete buf_recived;
    return distances;
}

//Function to get the hall sensor value on the Arduino
//sensor_number: 0 (Sensor 1), 1 (Sensor 2), 2 (Sensor 3)
//return positive int from 0 ~ 65535 on success, -1 on failure
int Arduino_Peripherals::get_hall_sensor_value(int sensor_number){
    uint8_t buf_send[2];
    buf_send[0] = 0b00000100;
    buf_send[1] = sensor_number;
    if (write(file, buf_send, 2) != 2) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at getting hall sensor value"<<std::endl;
        return -1;
    }

    uint8_t buf_recived[1];
    if (read(file, buf_recived, 1) != 1) {
        // I2C power on command failed to deliver
        std::cerr<<"Arduino_Peripherals error at getting hall sensor value"<<std::endl;
        return -1;
    } 
    else {
        return ( int )(buf_recived[0]);
    }
}

std::vector<int> Arduino_Peripherals::get_all_distance_sensor_value_one_by_one(int num_sensors){
    std::vector<int> distances(num_sensors,0);
    for(int i = 0 ; i < num_sensors ; i++){
        distances.at(i)  = get_distance_sensor_value(i);
    }
    return distances;
}