// Motor A Connections
int in1 = 4;
int in2 = 5;

// Motor B Connections
int in3 = 6;
int in4 = 7;

// Infrared sensor (digital output)
const int IR = 3;

// Ultrasonic sensor 
const int TRIG_PIN = 11;
const int ECHO_PIN = 12;

// Most IR obstacle modules output LOW when they detect something.
// If yours outputs HIGH on detection, change this to false.
const bool IR_ACTIVE_LOW = true;

// Ultrasonic detection range in cm (tune for your ring size)
const int DETECT_DISTANCE_CM = 40;

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

// Motor right Function (spin/turn right)
void motor_right(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// Motor backwards Function
void motor_backward(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

// IR Helper Function - true if the IR sensor sees something
bool ir_detects(){
  int reading = digitalRead(IR);
  return IR_ACTIVE_LOW ? (reading == LOW) : (reading == HIGH);
}

// Ultrasonic Helper Function - returns distance in cm (999 if no echo)
float get_ultrasonic_distance(){
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000);  // 25ms timeout keeps loop fast
  if (duration == 0) return 999;                    // no echo -> nothing detected

  return duration * 0.034 / 2.0;
}

void setup() {
  // Set all the motor control pins to outputs
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  // Sensor pins
  pinMode(IR, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(9600);

  // Sumo rules usually require a 5 second start delay
  delay(5000);
}

void loop() {
  bool ir_seen = ir_detects();
  float us_distance = get_ultrasonic_distance();

  Serial.print("IR: ");
  Serial.print(ir_seen ? "DETECT" : "clear");
  Serial.print(" | US: ");
  Serial.print(us_distance);
  Serial.println(" cm");

  bool opponentSeen = ir_seen || (us_distance < DETECT_DISTANCE_CM);

  if (opponentSeen) {
    motor_forward();   // Charge and push!
  } else {
    motor_right();     // Spin to search
  }

  delay(50);
}
