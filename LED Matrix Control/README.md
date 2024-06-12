1. install ImageMagick package
2. install curl package
3. install nlohmann json package
4. install libi2c-dev
sudo apt-get install libi2c-dev
5. run this 
sudo ldconfig /usr/local/lib
6. install libgpiod
sudo apt install libgpiod-dev

BH1750 Ambient Light Sensor 
Pin Layout
VCC -> 3.3V
GND -> GND
SCL -> SCL
SDA -> SDA
ADD -> if ( connected to GND ) { Address = 0x23 }, else if( connected to VCC with 10k resistor pullup) { Address = 0x5C } 

MMA845X Accelermometer 
Pin layout
VCC_IN -> 3.3V
3.3V -> None (This can output 3.3V up to 100mA, which is not used here)
GND -> GND
SCL -> SCL
SDA -> SDA
SAO -> if ( connected to GND ) { Address = 0x1C }, else if( connected to VCC with 10k resistor pullup) { Address = 0x1D }
INT1 -> advanced use, not utilized here
INT2 -> advanced use, not utilized here

SHT-30-D Digital temperature and humudity seneor
Pin Layout
VIN -> 3.3V
GND -> GND
SCL -> SCL
SDA -> SDA
AD -> if ( connected to GND ) { Address = 0x44 }, else if( connected to VCC with 10k resistor pullup) { Address = 0x45 } 
AL -> for advanced alarm use, not utilized here

EC11E Rotary Encoder with push-on switch
With knob pointing up and three legs close to your side, A C B
The other two legs D E are short when the button is pushed.


Thread management
1. Main thread int main():
    This thread is responsible for showing refreshing the display and showing the data on the screen. 
    Also, it has to manage the other threads.
2. API thread
    Called at the start of the main thread for updating weather data for the current location and time to avoid accessing a empty database. Remember to api_thread.join() to prevent accessing an empty database when this thread isn't yet complete at updating the database.

    Next, it may be called at any point of time to update the weather data for the current location and time. Designed to be called perhaps for every 30 minutes or so.

    
3. Sensor thread
    This thread updates all the sensor's value, storing them into std:atomic variables to be accessed by the main thread.
    This thread should also be initiated at the start of main thread, and should be ended by making std::atomic<bool> sensor_thread_control = false, then join thread to exit gracefully. 

Agreement on how to send data through FIFO(used in fifo_communacation.h, main.py)
    Send using  field_id:data_value'\n'  , for example below
1. For next page, id = 0
    "0:0"
2. For last page, id = 1
    "1:0"
3. For "page_ctrl", id = 2
    "2:0"
4. For "page_activation", id = 3
    "3:-2147483648"
5. For "location_id_ctrl", id = 4
    "4:0"
6. To terminate server
    "end:0"




Custom Pin layout for HUB75 to LED Matrix

3.3V
GPIO 2
GPIO 3
GPIO 4
GND
GPIO 17
GPIO 27
GPIO 22
3.3V
GPIO 10 MOSI
GPIO 9 MISO
GPIO 11 SCLK
GND
GPIO 0 (EEPROM SDA)
GPIO 5
GPIO 6
GPIO 13 (PWM1)
GPIO 19 (PCM FS)
GPIO 26
GND

Arduino acepts these custom commands
1. Power up : nothing has to be done
2. Power down : send bytes = [0b00000000], only one byte is needed
3. LED strip : 
    device_id = the "serial" number of the LED strip that you're trying to access 
    send bytes : [device_type_id = 1, device_id, pwm value for red, pwm value for green, pwm value for blue]
    e.g. change the brigntness of LED strip 0 , Red to 240, green to 0, blue to 100 : send bytes = [0b00000001, 0, 240, 0, 100]
4. Motor:
    device_id = the "serial" number of the motor that you're trying to access 
    selection = 0, 1 or 2. 0 for setting pwm value, 1 for setting time duration for the motor to run on ce started, 2 for setting the pwm value for which the motor runs on it's own.
    send bytes : [device_type_id = 2, device_id , selection , pwm value(0~255) or time duration value(in seconds from 0~2^32-1)]
    e.g. change the pwm of motor 0 to 240 : send bytes = [0b00000010, 0, 0, 240]
    e.g. change the time duration of motor 0 to 1000 seconds : send bytes = [0b00000010, 0, 1, 0,0,3,232]
    e.g. change the pwm which the motor runs on it's own for motor 0 to 240 : send bytes = [0b00000010, 0, 2, 240]
5. Distance Sensors(Ultrasonic):
    selection = 0 or 1, 0 for asking the number of distance sensors that are attached, 1 for accessing individual distance sensor values ,2 for retrieveing all the distance sensor's value
    send bytes if selection = 0 or 2: [device_type_id = 3, selection]
    send bytes if selection = 1: [device_type_id = 3, selection, device_id]
    recieve bytes if selection = 0 : [number of distance sensors] 
    recieve bytes if selection = 1 : [sensor value MSB, sensor value LSB] 
    recieve bytes if selection = 2 : [sensor 0 value MSB, sensor 0 value LSB, sensor 1 value MSB , sensor 2 value LSB...] with variable length equal to the number of distance sensors * 2 (MSB and LSB for one data occupies 2 bytes) 
    e.g. retireve the distance of the all Distance Sensors : send bytes = [0b00000011, 1], recieve bytes = [ s0 MSB, s0 LSB, s1 MSB , s1 LSB ...]
5. Hall Sensor:
    device_id = the "serial" number of the Hall Sensor that you're trying to access
    send bytes : [device_type_id = 4, device_id]
    e.g. retireve the distance of the first Hall Sensor : send bytes = [0b00000100, 0], recieve bytes = [ data ]
    



