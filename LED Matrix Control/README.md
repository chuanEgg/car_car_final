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
