#include <Wire.h>
#include "modules.h"

#define I2C_ADDRESS 3
#define led_strip_num 1
#define motor_num 2
#define hall_sensor_num 1
#define distance_sensor_num 3
#define max_command_length 10

#define LED_STRIP_RED 11
#define LED_STRIP_GREEN 10
#define LED_STRIP_BLUE 9

#define MOTOR_PIN 6
#define FAN_PIN 5

#define HALL_SENSOR_PIN 12
#define DISTANCE_SENSOR_TRIG_PIN 14
#define DISTANCE_SENSOR_ECHO_PIN_0 15
#define DISTANCE_SENSOR_ECHO_PIN_1 16
#define DISTANCE_SENSOR_ECHO_PIN_2 17

#define BUTTON_PIN 13


LED_Strip* led_strip_list[led_strip_num];
Motor* motor_list[motor_num];
Hall_Sensor* hall_sensor_list[hall_sensor_num];
Button* button;
double* distances;
uint8_t command[max_command_length];
// Distance_Sensor* distance_sensor_list[distance_sensor_num];

unsigned long button_pressed_time = 0;

bool motor_on = false;



void receiveEvent(int numBytes){
  // Serial.println("Recieved I2C command!");
  for(int i = 0 ; i < numBytes; i++){
    command[i] = 0;
    if(Wire.available()){
      command[i] = Wire.read();
      // Serial.print((int)command[i]);
      // Serial.print(' ');
    }
  }
  Serial.print("\n");
  switch((int)command[0]){
    case 0:
      for(int i = 0 ; i < led_strip_num; i++){
        led_strip_list[i]->set_pwm(0,0,0);
      }
      for(int i = 0 ; i < motor_num; i++){
        motor_list[i]->set_pwm(0);
      }
      break;
      
    case 1:
      if(command[1] < led_strip_num){
        led_strip_list[(int)command[1]]->set_pwm((int)command[2],(int)command[3],(int)command[4]);
        // Serial.println("LED set by i2c command");
      }
      break;
    case 2:
      for(int i = 0 ; i < numBytes; i++){
        Serial.print(command[i]);
        Serial.print(' ');
      }
      Serial.print('\n');
      switch((int)command[2]){
        case 0:
          motor_list[(int)command[1]]->set_pwm((int)command[3]);
          break;
        case 1:
          motor_list[(int)command[1]]->duration = ( (command[3] << 24) + (command[4] << 16) + (command[5] << 8) + command[6] )*1000;
          break;
        case 2:
          motor_list[(int)command[1]]->pwm_value = ((int)command[3]);
          break;
      }
      Serial.print(motor_list[0]->pwm_value);
      Serial.print(' ');
      Serial.print(motor_list[1]->pwm_value);
      Serial.print('\n');
      break;

    default:
      break;
  }
}

void requestEvent(){

  switch((int)command[0]){
    case 3:
      switch((int)command[1]){
        case 0:
          Wire.write(distance_sensor_num);
          break;
        case 1:
          uint8_t distance_value[2];
          //ensure that value isn't negative
          if(distances[(int)command[2]] < 0){
            distances[(int)command[2]] = 0;
          }
          distance_value[0] = (uint8_t) ( ( (int)( distances[(int)command[2]] ) ) / 256 );
          distance_value[1] = (uint8_t) ( ( (int)( distances[(int)command[2]] ) ) % 256 );
          Wire.write(distance_value,(size_t)2);
          break;
        case 2:
          uint8_t* distance_values = new uint8_t[distance_sensor_num*2+1];
          // Serial.println("data sent:");
          for(int i = 0 ; i < distance_sensor_num ; i++){
            //ensure that value isn't negative
            if(distances[i] < 0){
              distances[i] = 0;
            }
            distance_values[i*2] = (uint8_t) ( ( (int)( distances[i] ) ) / 256 );
            distance_values[i*2+1] = (uint8_t) ( ( (int)( distances[i] ) ) % 256 );
            // Serial.print((int)distance_values[i*2]);
            // Serial.print(' ');
            // Serial.print((int)distance_values[i*2+1]);
            // Serial.print(' ');
          }
          // Serial.print('\n');
          Wire.write(distance_values,(size_t)distance_sensor_num*2);
          delete distance_values;
          break;
      }
      break;
    case 4:
      if(command[1] < hall_sensor_num){
        uint8_t value = (uint8_t) hall_sensor_list[(int)command[1]]->get_value();
        Wire.write(value);
      }
      break;

    default:
      break;
  }
  for(int i = 0 ; i < max_command_length ; i++){
    command[i] = 0;
  }
}




void setup(){
  Wire.begin(I2C_ADDRESS); //set address of arduino for i2c 
  Wire.onReceive(receiveEvent); //Called when recieved I2C data
  Wire.onRequest(requestEvent);
  Serial.begin(9600); 
  motor_list[0] = new Motor(MOTOR_PIN);
  motor_list[1] = new Motor(FAN_PIN);
  led_strip_list[0] = new LED_Strip(LED_STRIP_RED,LED_STRIP_GREEN,LED_STRIP_BLUE);
  
  hall_sensor_list[0] = new Hall_Sensor(HALL_SENSOR_PIN);
  // distance_sensor_list[0] = new Distance_Sensor(DISTANCE_SENSOR_TRIG_PIN,DISTANCE_SENSOR_ECHO_PIN);
  byte* distance_sensors_echo_pin_list = new byte[distance_sensor_num] {DISTANCE_SENSOR_ECHO_PIN_0, DISTANCE_SENSOR_ECHO_PIN_1, DISTANCE_SENSOR_ECHO_PIN_2};
  HCSR04.begin( (byte) DISTANCE_SENSOR_TRIG_PIN , distance_sensors_echo_pin_list , (byte) distance_sensor_num );

  button = new Button(BUTTON_PIN);

  for(int i = 0 ; i < max_command_length ; i++){
    command[i] = 0;
  }
}

void loop(){
  // testing
  // motor_list[0]->set_pwm(255);
  // for(int i = 0 ; i < 32 ; i++){
  //   led_strip_list[0]->set_pwm(i*8,0,0);
    distances = HCSR04.measureDistanceCm();
    // Serial.println(hall_sensor_list[0]->get_value());
    // Serial.print(distances[0]);
    // Serial.print(" ");
    // Serial.print(distances[1]);
    // Serial.print(" ");
    // Serial.print(distances[2]);
    // Serial.print("--------\n");
    // delay(300);
  

  // for(int i = 0 ; i < 32 ; i++){
  //   led_strip_list[0]->set_pwm(i*8,0,0);
  //   delay(100);
  // }
  // for(int i = 0 ; i < 32 ; i++){
  //   led_strip_list[0]->set_pwm(256-i*8,0,0);
  //   delay(100);
  // }
  // for(int i = 0 ; i < 32 ; i++){
  //   led_strip_list[0]->set_pwm(0,i*8,0);
  //   delay(100);
  // }
  // for(int i = 0 ; i < 32 ; i++){
  //   led_strip_list[0]->set_pwm(0,256-i*8,0);
  //   delay(100);
  // }
  // for(int i = 0 ; i < 32 ; i++){
  //   led_strip_list[0]->set_pwm(0,0,i*8);
  //   delay(100);
  // } 
  // for(int i = 0 ; i < 32 ; i++){
  //   led_strip_list[0]->set_pwm(0,0,256-i*8);
  //   delay(100);
  // } 
  // Serial.print("going\n");
  // led_strip_list[0]->set_pwm(0,0,0);


  // if user pressed button
  if(button->get_value() && millis() - button_pressed_time > 200){
    button_pressed_time = millis();
    Serial.println("button pressed");
    bool is_running = false;
    for(int i = 0 ; i < motor_num ; i++){
      is_running = is_running || ( motor_list[i]->current_pwm > 0 );
    }

    if(is_running){  // if user pressed the button again before the motor stops (with 200ms spacing to prevent switch bounce), it's treated as a emergency stop which fan and motor stops
      for(int i = 0 ; i < motor_num ; i++){
        motor_list[i]->set_pwm(0);
        //for testing
        led_strip_list[0]->set_pwm(0,0,0);
        //for testing
      }
    }
    else{ //if user pressed button when no other motor is on, motor and fan turns on.
      for(int i = 0 ; i < motor_num ; i++){
        motor_list[i]->set_pwm( motor_list[i]->pwm_value );
        motor_list[i]->start_time = millis();

        //for testing
        led_strip_list[0]->set_pwm(255,0,0);
        Serial.println("start to run");
        //for testing
      }
    }

  }
  // after timeout the motor and fan stops automatically
  for(int i = 0 ; i < motor_num ; i++){
    if( motor_list[i]->current_pwm > 0 && millis() - motor_list[i]->start_time > motor_list[i]->duration ){
        motor_list[i]->set_pwm(0);
        //for testing
        // led_strip_list[0]->set_pwm(0,0,0);
        Serial.print("exited\n");
        //for testing
      }
  }

  // if(button->get_value() &&  motor_on && (millis() - motor_list[0]->start_time > 200)){
  //   motor_on = false;
  //   motor_list[0]->set_pwm(0);
  //   motor_list[1]->set_pwm(0);
  // }

  // if( motor_on && (millis() - motor_list[0]->start_time > motor_list[0]->duration ) ){
  //   motor_on = false;
  //   motor_list[0]->set_pwm(0);
  //   motor_list[1]->set_pwm(0);
  // }

  // if(button->get_value() && millis() - button_pressed_time > 200){
  //   button_pressed_time = millis();
  //   Serial.println("button pressed");

  //   if(motor_on == false){
  //     motor_list[0]->set_pwm( motor_list[0]->pwm_value );
  //     motor_list[1]->set_pwm( motor_list[0]->pwm_value );
  //   }
  // }
}

