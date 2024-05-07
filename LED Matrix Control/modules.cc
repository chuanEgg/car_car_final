#include "modules.h"

int initialize_chip(){
    chip = gpiod_chip_open_by_name(chipname);
    if (!chip) {
        std::cerr << "Failed to open " << chipname << std::endl;
        return 1;
    }
    return 0;
}

int close_chip(){
    gpiod_chip_close(chip);
    return 0;
}

EC11E_Mode& operator++(EC11E_Mode& mode){
    switch(mode){
        case NO_ROTATION:
            mode = NO_ROTATION_SWITCH_PRESSED;
            break;
        case NO_ROTATION_SWITCH_PRESSED:
            mode = CLOCKWISE;
            break;
        case CLOCKWISE:
            mode = CLOCKWISE_SWITCH_PRESSED;
            break;
        case CLOCKWISE_SWITCH_PRESSED:
            mode = COUNTER_CLOCKWISE;
            break;
        case COUNTER_CLOCKWISE:
            mode = COUNTER_CLOCKWISE_SWITCH_PRESSED;
            break;
        case COUNTER_CLOCKWISE_SWITCH_PRESSED:
            mode = NO_ROTATION;
            break;
        default:
            std::cerr<<"EC11E error at incrementing mode"<<std::endl;
            break;
    }
    return mode;
}

EC11E_Mode operator++(EC11E_Mode& mode, int num){
    for(int i = 0 ; i < num ; i++){
        ++mode;
    }
    return mode;
}

BH1750::BH1750(ADDRESS_SELECT type){
    switch(type){
        case GND:
            device_address = 0x23;
            break;
        case VCC:
            device_address = 0x5C;
            break;
        default:
            std::cout<<"BH1750 error at determining I2C device address"<<std::endl;
            break;
    }
    file = open(filename, O_RDWR);
    if (file < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
        std::cout<<"BH1750 error at opening file"<<std::endl;
    }
    if (ioctl(file, I2C_SLAVE, device_address) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
        std::cout<<"BH1750 error at initializing"<<std::endl;
    }

    usleep(1);

    //little endian
    //power on command
    char buf_send[1];
    buf_send[0] = 0b00000001;
    if (write(file, buf_send, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"BH1750 error at power on"<<std::endl;
    }
}

BH1750::~BH1750(){
    //little endian
    //power down command    
    char buf_send[1];
    buf_send[0] = 0b00000000;
    if (write(file, buf_send, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"BH1750 error at power down"<<std::endl;
    }
    close(file);
}

int BH1750::get_lux(){
    char buf_recived[2];
    if (read(file, buf_recived, 2) != 2) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"BH1750 error at reading lux"<<std::endl;
        return -1;
    } 
    else {
        int lux = buf_recived[0] << 8 | buf_recived[1];
        return lux;
    }
}

int BH1750::change_mode(BH1750_Mode mode){
    char buf_send[1];
    buf_send[0] = mode;
    if (write(file, buf_send, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"BH1750 error at changing to mode : "<<std::bitset<8>(mode)<<std::endl;
        return -1;
    }
    else{
        return 1;
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
            std::cout<<"BH1750 error at determining I2C device address"<<std::endl;
            break;
    }
    file = open(filename, O_RDWR);
    if (file < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
        std::cout<<"MMA8452 error at opening file"<<std::endl;
    }
    if (ioctl(file, I2C_SLAVE, device_address) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
        std::cout<<"MMA8452 error at initializing"<<std::endl;
    }

    usleep(1);

    //little endian
    //power on command
    char buf_send[2];
    buf_send[0] = 0x2A;//register address
    buf_send[1] = 1;//data 
    if (write(file, buf_send, 2) != 2) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"MMA8452 error at initializing"<<std::endl;
    }
}

MMA8452::~MMA8452(){
    //little endian
    //power down command
    char buf_send[2];
    buf_send[0] = 0x2A;//register address
    buf_send[1] = 0;//data 
    if (write(file, buf_send, 2) != 2) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"MMA8452 error at powering down"<<std::endl;
    }
    close(file);
}

std::vector<int> MMA8452::get_acceleration(){
    std::vector<int> acceleration(3,INT_MIN);
    char buf_send[1];
    char buf_recived[4];
    buf_send[0] = 0x01;//register address
    if (write(file, buf_send, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"MMA8452 error at reading acceleration"<<std::endl;
        return acceleration;
    }
    else{
        if (read(file, buf_recived, 4) != 4 ) {
        /* ERROR HANDLING: i2c transaction failed */
            std::cout<<"MMA8452 error at reading acceleration"<<std::endl;
            return acceleration;
        } 
        else {
            acceleration.at(0) = ( buf_recived[0] << 8 | buf_recived[1] ) >> 4 ;
            acceleration.at(1) = ( buf_recived[2] << 8 | buf_recived[3] ) >> 4 ;
        }
    }

    buf_send[0] = 0x05;//register address
    if (write(file, buf_send, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"MMA8452 error at reading acceleration"<<std::endl;
        return acceleration;
    }
    else{
        if (read(file, buf_recived, 4) != 4) {
        /* ERROR HANDLING: i2c transaction failed */
            std::cout<<"MMA8452 error at reading acceleration"<<std::endl;
            return acceleration;
        } 
        else {
            acceleration.at(2) = ( buf_recived[0] << 8 | buf_recived[1] ) >> 4 ;
            return acceleration;
        }
    }

}

EC11E::EC11E(int pinA, int pinB, int pin_Switch){
    bool ok = true;
    this->pinA = pinA;
    this->pinB = pinB;
    this->pin_Switch = pin_Switch;

    timeout.tv_sec = 0;
    timeout.tv_nsec = 0;   

    if( !( pinA == 5 || pinA == 6 || pinA == 12 || pinA == 13 || pinA == 16 || pinA == 17 || pinA == 18 || pinA == 19 || pinA == 20 || pinA == 21 || pinA == 22 || pinA == 23 || pinA == 24 || pinA == 25 || pinA == 26 || pinA == 27 ) ){
        std::cout<<"EC11E error at pinA in pin assignment"<<std::endl;
        ok = false;
    }
    if( !( pinB == 5 || pinB == 6 || pinB == 12 || pinB == 13 || pinB == 16 || pinB == 17 || pinB == 18 || pinB == 19 || pinB == 20 || pinB == 21 || pinB == 22 || pinB == 23 || pinB == 24 || pinB == 25 || pinB == 26 || pinB == 27 ) ){
        std::cout<<"EC11E error at pinB in pin assignment"<<std::endl;
        ok = false;
    }
    if( !( pin_Switch == 5 || pin_Switch == 6 || pin_Switch == 12 || pin_Switch == 13 || pin_Switch == 16 || pin_Switch == 17 || pin_Switch == 18 || pin_Switch == 19 || pin_Switch == 20 || pin_Switch == 21 || pin_Switch == 22 || pin_Switch == 23 || pin_Switch == 24 || pin_Switch == 25 || pin_Switch == 26 || pin_Switch == 27 ) ){
        std::cout<<"EC11E error at pin_Switch in pin assignment"<<std::endl;
        ok = false;
    }
    if(pinA == pinB || pinA == pin_Switch || pinB == pin_Switch){
        std::cout<<"EC11E error at pin assignment, repeated pin assignment "<<std::endl;
        ok = false;
    }

    if(ok){
        //Open GPIO lines
        A = gpiod_chip_get_line(chip, pinA);
        B = gpiod_chip_get_line(chip, pinB);
        Button = gpiod_chip_get_line(chip, pin_Switch);

        //Request line event for rising and falling edge
        int ret_A = gpiod_line_request_falling_edge_events(A, "EC11E Rotary Encoder");
        int ret_B = gpiod_line_request_falling_edge_events(B, "EC11E Rotary Encoder");
        //int ret_Button = gpiod_line_request_both_edge_events(Button, "EC11E Rotary Encoder");
        int ret_Button = gpiod_line_request_input(Button, "EC11E Rotary Encoder");

        //Check if request is successful
        if(ret_A < 0){
            std::cerr<<"EC11E error at requesting line event for pinA"<<std::endl;
        }
        if(ret_B < 0){
            std::cerr<<"EC11E error at requesting line event for pinB"<<std::endl;
        }
        if(ret_Button < 0){
            std::cerr<<"EC11E error at requesting line event for pin_Switch"<<std::endl;
        }  
    }
}

EC11E::~EC11E(){
    gpiod_line_release(A);
    gpiod_line_release(B);
    gpiod_line_release(Button);
}

EC11E_Mode EC11E::get_rotation(){
    //int A_value = gpiod_line_get_value(A);
    //int B_value = gpiod_line_get_value(B);
    //int Button_value = gpiod_line_get_value(Button);

    EC11E_Mode mode = NO_ROTATION;
    int ret_A = gpiod_line_event_wait(A, &timeout);
    int ret_B = gpiod_line_event_wait(B, &timeout);
    //int ret_Button = gpiod_line_event_wait(Button, &timeout);

    if(ret_A < 0){
        std::cerr<<"EC11E error at waiting for line event for pinA"<<std::endl;
        return NO_ROTATION;
    }
    if (ret_B < 0){
        std::cerr<<"EC11E error at waiting for line event for pinB"<<std::endl;
        return NO_ROTATION;
    }
    /*if(ret_Button < 0){
        std::cerr<<"EC11E error at waiting for line event for pin_Switch"<<std::endl;
        return NO_ROTATION;
    }*/

    if(ret_A > 0 && ret_B > 0){
        if( gpiod_line_event_read(A, &event_A) == 0  && gpiod_line_event_read(B, &event_B) == 0){
            if(event_A.ts.tv_sec > event_B.ts.tv_sec){
                mode = CLOCKWISE;
            }
            else if(event_A.ts.tv_sec == event_B.ts.tv_sec){
                if(event_A.ts.tv_nsec > event_B.ts.tv_nsec){
                    mode = CLOCKWISE;
                }
                else{
                    mode = COUNTER_CLOCKWISE;
                }
            }
            else{
                mode = COUNTER_CLOCKWISE;
            }
        }
        else{
            std::cerr<<"EC11E error at reading line event for pinA or pinB"<<std::endl;
        }
    }
    else if(ret_A > 0 && ret_B == 0){
        if( gpiod_line_event_read(A, &event_A) != 0 ){
            std::cerr<<"EC11E error at reading line event for pinA or pinB"<<std::endl;
        }
        mode = CLOCKWISE;
    }
    else if(ret_A == 0 && ret_B > 0){
        if( gpiod_line_event_read(B, &event_B) != 0 ){
            std::cerr<<"EC11E error at reading line event for pinA or pinB"<<std::endl;
        }
        mode = COUNTER_CLOCKWISE;
    }

    //Check if button is pressed
    if(gpiod_line_get_value(Button)){
        ++mode;
    }

    std::cout<<ret_A<<" "<<ret_B<<" "<<mode<<std::endl;


    


    /*
    if(Last_A_value != A_value){
        if(A_value != B_value){
            if(Button_value == 0){
                mode = CLOCKWISE;
            }
            else{
                mode = CLOCKWISE_SWITCH_PRESSED;
            }
        }
        else{
            if(Button_value == 0){
                mode = COUNTER_CLOCKWISE;
            }
            else{
                mode = COUNTER_CLOCKWISE_SWITCH_PRESSED;
            }
        }
        Last_A_value = A_value;
    }
    else{
        if(Button_value == 0){
            mode = NO_ROTATION;
        }
        else{
            mode = NO_ROTATION_SWITCH_PRESSED;
        }
    }
    */
    return mode;
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
            std::cout<<"SHT30 error at determining I2C device address"<<std::endl;
            break;
    }
    file = open(filename, O_RDWR);
    if (file < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
        std::cout<<"SHT30 error at opening file"<<std::endl;
    }
    if (ioctl(file, I2C_SLAVE, device_address) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
        std::cout<<"SHT30 error at initializing"<<std::endl;
    }

    

    //little endian
    //command for setting up high repeatable 0.5 measurments per second mode
    char buf_send[2];
    buf_send[0] = 0x20;//MSB
    buf_send[1] = 0x32;//LSB 
    if (write(file, buf_send, 2) != 2) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"SHT30 error at initializing"<<std::endl;
    }
    
    usleep(15); //wait for at least 15 ms to complete first measurement (datasheet page 10
}

SHT30::~SHT30(){
    //little endian
    //command for canceling constant measuring 
    char buf_send[2];
    buf_send[0] = 0x30;//MSB
    buf_send[1] = 0x93;//LSB 
    if (write(file, buf_send, 2) != 2) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"SHT30 error at closing"<<std::endl;
    }
    close(file);
}

std::vector<double> SHT30::get_temperature_humidity(){
    static std::vector<double> temperature_humidity(2,INT_MIN);
    char buf_send[2];
    char buf_recived[6];
    buf_send[0] = 0xE0;
    buf_send[1] = 0x00;
    if (write(file, buf_send, 2) != 2) {
    /* ERROR HANDLING: i2c transaction failed */
        //std::cout<<"SHT30 error at reading temperature and humidity"<<std::endl;
        return temperature_humidity;
    }
    else{
        if (read(file, buf_recived, 6) != 6) {
        /* ERROR HANDLING: i2c transaction failed */
            //std::cout<<"SHT30 error at reading temperature and humidity"<<std::endl;
            return temperature_humidity;
        } 
        else {
            temperature_humidity.at(0) = ( buf_recived[0] << 8 | buf_recived[1] ) * 175 / 65535 - 45;
            temperature_humidity.at(1) = ( buf_recived[3] << 8 | buf_recived[4] ) * 100 / 65535;
            return temperature_humidity;
        }
    }
}

Push_Button::Push_Button(int pin){
    this->pin = pin;
    Button = gpiod_chip_get_line(chip, pin);
    int ret_Button = gpiod_line_request_input(Button, "EC11E Rotary Encoder");
    if(ret_Button < 0){
        std::cerr<<"Push_Button error at requesting line event for pin"<<std::endl;
    }
}

Push_Button::~Push_Button(){
    gpiod_line_release(Button);
}

bool Push_Button::get_state(){
    return gpiod_line_get_value(Button);
}