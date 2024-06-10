#include <Wire.h>
#include <Ultrasonic.h>


class LED_Strip{
  public:
    //Initializer for LED_Strip
    //Recommended to use Analog pins or digital piins with PWM
    LED_Strip(int pinRed, int pinBlue, int pinGreen):pinR(pinRed),pinG(pinGreen),pinB(pinBlue){
      pinMode(pinR,OUTPUT);
      pinMode(pinG,OUTPUT);
      pinMode(pinB,OUTPUT);

      analogWrite(pinR,0);
      analogWrite(pinG,0);
      analogWrite(pinB,0);
    }

    ~LED_Strip(){
      analogWrite(pinR,0);
      analogWrite(pinG,0);
      analogWrite(pinB,0);
    }

    void set_pwm(int pwmR, int pwmG, int pwmB){
      analogWrite(pinR,pwmR);
      analogWrite(pinG,pwmG);
      analogWrite(pinB,pwmB);
    }
    
  private:
    int pinR;
    int pinG;
    int pinB;
};

class Motor{
  public:
    //Initializer for Motor
    //Recommended to use Analog pins or digital piins with PWM
    Motor(int num):pin_num(num){
      pinMode(pin_num,OUTPUT);
      analogWrite(pin_num,0);
    }

    ~Motor(){
      analogWrite(pin_num,0);
    }

    void set_pwm(int num){
      num = num < 256 ? num : 255;
      num = num >= 0 ? num : 0;
      analogWrite(pin_num,num);
    }
    
  private:
    int pin_num;
};

class Distance_Sensor{
  public:
    Distance_Sensor(int Trig, int Echo){
      distance_sensor = new Ultrasonic(Trig, Echo);
    }
    ~Distance_Sensor(){
      delete distance_sensor;
    }
    int get_distance(){
      distance = distance_sensor->read();
      return distance;
    }
  private:
    Ultrasonic* distance_sensor;
    int distance;
};

class Hall_Sensor{
  public:
    Hall_Sensor(int pin_num):pin(pin_num){
      pinMode(pin,INPUT);
    }
    int get_value(){
      return digitalRead(pin);
    }
  private:
    int pin;
};