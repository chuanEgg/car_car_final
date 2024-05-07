#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "include/led-matrix.h"
#include <Magick++.h>

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>


int file;
int adapter_number = 1;
char filename[20] = "/dev/i2c-1";
int device_address = 0x23;


int main(){
    std::cout<<"hello world"<<std::endl;
    unsigned char buf_send[1];
    unsigned char buf_recived[2];
    file = open(filename, O_RDWR);
    if (file < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    std::cout<<"error at 1"<<std::endl;
        return 1;
    }
    if (ioctl(file, I2C_SLAVE, device_address) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
        return 1;
    }

    usleep(10);

    //little endian
    //power on command
    buf_send[0] = 0b00000001;
    if (write(file, buf_send, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"error at power on"<<std::endl;
        return 1;
    }

    usleep(10);

    //little endian
    //One Time H-Resolution Mode
    buf_send[0] = 0b00010000;
    if (write(file, buf_send, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
        std::cout<<"error at One Time H-Resolution Mode"<<std::endl;
        return 1;
    }

    usleep(10);

    while(true){
        if (read(file, buf_recived, 2) != 2) {
        /* ERROR HANDLING: i2c transaction failed */
            std::cout<<"error at 3"<<std::endl;
            return 1;
        } 
        else {
        /* buf[0] contains the read byte */
            int lux = (int)buf_recived[0]*256+(int)buf_recived[1];
            std::cout<<lux<<std::endl;
            //if((int)buf[0]!=0){std::cout<<(int)buf[0]<<std::endl;}
        }
        sleep(1);        
    }

    return 0;
}