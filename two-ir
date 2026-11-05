/*
 * Behavior:
 *  - Both sensors detect  -> opponent is dead ahead, charge forward
 *  - Only LEFT detects    -> turn left toward the opponent
 *  - Only RIGHT detects   -> turn right toward the opponent
 *  - Neither detects      -> spin right to search
 */

// Motor A Connections
int in1 = 4;
int in2 = 5;

// Motor B Connections
int in3 = 6;
int in4 = 7;

// Left IR sensor (digital output)
const int IR_LEFT = 3;

// Right IR sensor (digital output)
const int IR_RIGHT = 2;

// Most IR obstacle modules output LOW when they detect something.
// If yours outputs HIGH on detection, change this to false.
const bool IR_ACTIVE_LOW = true;

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

// Motor right Function (motor A on, motor B off -> arcs right)
void motor_right(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// Motor left Function (motor B on, motor A off -> arcs left)
void motor_left(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

// Motor backwards Function
void motor_backward(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

// IR Helper Function - true if the given IR sensor sees something
bool ir_detects(int pin){
  int reading = digitalRead(pin);
  return IR_ACTIVE_LOW ? (reading == LOW) : (reading == HIGH);
}

void setup() {
  // Set all the motor control pins to outputs
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  // Sensor pins
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);

  Serial.begin(9600);

  // Sumo rules usually require a 5 second start delay
  delay(5000);
}

void loop() {
  bool left_seen  = ir_detects(IR_LEFT);
  bool right_seen = ir_detects(IR_RIGHT);

  Serial.print("Left: ");
  Serial.print(left_seen ? "DETECT" : "clear");
  Serial.print(" | Right: ");
  Serial.println(right_seen ? "DETECT" : "clear");

  if (left_seen && right_seen) {
    motor_forward();      // Opponent dead ahead - charge!
  } else if (left_seen) {
    motor_left();         // Opponent to the left - turn into it
  } else if (right_seen) {
    motor_right();        // Opponent to the right - turn into it
  } else {
    motor_right();        // Nothing seen - spin to search
  }

  delay(50);
}
