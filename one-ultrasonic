#include <Wire.h>

// Motor A Connections
int in3 = 4;
int in4 = 5;

// Motor B Connections
int in1 = 6;
int in2 = 7;

// Ultrasonic sensor 
int trig_pin = 9;
int echo_pin = 8;

// Motor Forward Function
void motor_forward(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

// Motor Stop Function
void motor_stop(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// Motor right Function
void motor_right(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// Ultrasonic Helper Functions
float getDistance() {
  // Clear the trigger
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);

  // Send 10us pulse
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  // Read echo pulse duration (timeout at 30000us = ~400cm)
  long duration = pulseIn(echo_pin, HIGH, 30000);

  if (duration == 0) return -1; // No echo = out of range

  // Convert to cm: sound travels 343m/s = 0.0343 cm/us
  // Divide by 2 for round trip
  return (duration * 0.0343) / 2.0;
}

// Motor backwards Function
void motor_backward(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}
 
void setup() {
  // Set all the motor control pins to outputs
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  Serial.begin(9600);
  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
}

void loop() {

  motor_right();
  delay(20);
  float distance = getDistance();
  Serial.println(distance);
  if (distance < 40){
    motor_forward();
    delay(50);
  }
}
