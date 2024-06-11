#include <Wire.h>
#include "modules.h"

#define I2C_ADDRESS 3
#define led_strip_num 1
#define motor_num 1
#define hall_sensor_num 1
#define distance_sensor_num 2

#define LED_STRIP_RED 11
#define LED_STRIP_GREEN 10
#define LED_STRIP_BLUE 9

#define MOTOR_PIN 6
#define FAN_PIN 5

#define HALL_SENSOR_PIN 12
#define DISTANCE_SENSOR_TRIG_PIN 8
#define DISTANCE_SENSOR_ECHO_PIN_0 7
#define DISTANCE_SENSOR_ECHO_PIN_1 4

#define BUTTON_PIN 13


LED_Strip* led_strip_list[led_strip_num];
Motor* motor_list[motor_num];
Hall_Sensor* hall_sensor_list[hall_sensor_num];
Button* button;
// Distance_Sensor* distance_sensor_list[distance_sensor_num];

bool motor_on;
unsigned long motor_start_time;
unsigned long motor_duration = 10000;

void setup(){
  Wire.begin(I2C_ADDRESS); //set address of arduino for i2c 
  Wire.onReceive(receiveEvent); //Called when recieved I2C data
  Serial.begin(9600); 

  led_strip_list[0] = new LED_Strip(LED_STRIP_RED,LED_STRIP_GREEN,LED_STRIP_BLUE);
  motor_list[0] = new Motor(MOTOR_PIN);
  hall_sensor_list[0] = new Hall_Sensor(HALL_SENSOR_PIN);
  // distance_sensor_list[0] = new Distance_Sensor(DISTANCE_SENSOR_TRIG_PIN,DISTANCE_SENSOR_ECHO_PIN);
  byte* distance_sensors_echo_pin_list = new byte[distance_sensor_num] {DISTANCE_SENSOR_ECHO_PIN_0, DISTANCE_SENSOR_ECHO_PIN_1};
  HCSR04.begin( (byte) DISTANCE_SENSOR_TRIG_PIN , distance_sensors_echo_pin_list , (byte) distance_sensor_num );

  button = new Button(BUTTON_PIN);
  motor_on = false;
}

void loop(){
  // testing
  // motor_list[0]->set_pwm(255);
  // for(int i = 0 ; i < 32 ; i++){
  //   led_strip_list[0]->set_pwm(255,i*8,255);
    
  //   Serial.println(hall_sensor_list[0]->get_value());
  //   double* distances = HCSR04.measureDistanceCm();
  //   Serial.print(distances[0]);
  //   Serial.print(" ");
  //   Serial.print(distances[1]);
  //   Serial.print("--------\n");
  //   delay(1000);
  // }

  if( !motor_on && button->get_value() > 0){
    motor_on = true;
    motor_start_time = millis();
    motor_list[0]->set_pwm(255);
    led_strip_list[0]->set_pwm(255,255,255);
    Serial.println("button pressed");
  }

  if( motor_on && (millis() - motor_start_time > motor_duration) ){
    motor_on = false;
    motor_list[0]->set_pwm(0);
    led_strip_list[0]->set_pwm(0,0,0);
  }
}

void receiveEvent(int numBytes){
  char* data[numBytes];
  for(int i = 0 ; i < numBytes; i++){
    data[i] = 0;
    if(Wire.available()){
      data[i] = Wire.read();
    }
  }

  switch((int)data[0]){
    case 0:
      for(int i = 0 ; i < led_strip_num; i++){
        led_strip_list[i]->set_pwm(0,0,0);
      }
      for(int i = 0 ; i < motor_num; i++){
        motor_list[i]->set_pwm(0);
      }
      break;
      
    case 1:
      if(data[1] < led_strip_num){
        led_strip_list[(int)data[1]]->set_pwm((int)data[2],(int)data[3],(int)data[4]);
      }
      break;

    case 2:
      if(data[1] < motor_num){
        motor_list[(int)data[1]]->set_pwm((int)data[2]);
      }
      break;

    case 3:
      if(data[1] < distance_sensor_num){
        Wire.write(hall_sensor_list[(int)data[1]]->get_value());
      }
      break;
    
    case 4:
      if(data[1] < hall_sensor_num){
        int value = hall_sensor_list[(int)data[1]]->get_value();
        char* data;
        data = new char[2];
        data[0] = (char)(value >> 8);
        data[1] = (char)(value % 256);
        Wire.write(data,(size_t)2);
      }
      break;

    default:
      break;
  }
}

