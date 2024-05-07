#ifndef MODULES_H
#define MODULES_H

#include <iostream>
#include <string>
#include <bitset>
#include <climits>
#include <vector>

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#include <gpiod.h>

const char filename[20] = "/dev/i2c-1";
const char chipname[20] = "gpiochip0";
extern struct gpiod_chip *chip;

enum ADDRESS_SELECT{
    GND,
    VCC
};

enum BH1750_Mode{
    POWER_DOWN = 0b00000000,
    POWER_ON = 0b00000001,
    RESET = 0b00000111,
    CONTINUOUSLY_H_RESOLUTION_MODE = 0b00010000,
    CONTINUOUSLY_H_RESOLUTION_MODE2 = 0b00010001,
    CONTINUOUSLY_L_RESOLUTION_MODE = 0b00010011,
    ONE_TIME_H_RESOLUTION_MODE = 0b00100000,
    ONE_TIME_H_RESOLUTION_MODE2 = 0b00100001,
    ONE_TIME_L_RESOLUTION_MODE = 0b00100011
};

enum EC11E_Mode{
    NO_ROTATION,
    NO_ROTATION_SWITCH_PRESSED,
    CLOCKWISE,
    CLOCKWISE_SWITCH_PRESSED,
    COUNTER_CLOCKWISE,
    COUNTER_CLOCKWISE_SWITCH_PRESSED,
};

EC11E_Mode& operator++(EC11E_Mode& mode);

EC11E_Mode operator++(EC11E_Mode& mode, int num);

class BH1750{
    public:
        BH1750(ADDRESS_SELECT type);
        ~BH1750();
        int change_mode(BH1750_Mode mode);//returns 1 on success, -1 on failure
        int get_lux();//return positive int from 0 ~ 65535 onsuccess, -1 on failure
    private:
        int file;
        int adapter_number = 1;
        //char filename[20] = "/dev/i2c-1";
        int device_address;
};

class MMA8452{
    public:
        MMA8452(ADDRESS_SELECT type);
        ~MMA8452();
        std::vector<int> get_acceleration();//return a vector of 3 double from 0 ~ 65535 on success, -1 on failure
    private:
        int file;
        int adapter_number = 1;
        //char filename[20] = "/dev/i2c-1";
        int device_address;
};

class EC11E{
    public:
        EC11E(int pinA, int pinB, int pin_Switch);
        ~EC11E();
        EC11E_Mode get_rotation();//return EC11_Mode
    private:
        int pinA, pinB, pinC, pin_Switch;//Pins that are connected to the Raspberry Pi 4
        struct gpiod_line *A; // A terminal
        struct gpiod_line *B; // B terminal
        struct gpiod_line *Button; // Pushbutton
        struct gpiod_line_event event_A;
        struct gpiod_line_event event_B;
        struct gpiod_line_event event_Button;
        struct timespec timeout;
};

class SHT30{
    public:
        SHT30(ADDRESS_SELECT type);
        ~SHT30();
        std::vector<double> get_temperature_humidity();//return a vector of 2 double from 0 ~ 65535 on success, -1 on failure
    private:
        int file;
        int adapter_number = 1;
        //char filename[20] = "/dev/i2c-1";
        int device_address;
};

class Push_Button{
    public:
        Push_Button(int pin);
        ~Push_Button();
        bool get_state();//return true if the button is pressed, false if not
    private:
        int pin;
        struct gpiod_line *Button; // Pushbutton
        struct gpiod_line_event event_Button;
        struct timespec timeout;
};

int initialize_chip();

int close_chip();



#endif