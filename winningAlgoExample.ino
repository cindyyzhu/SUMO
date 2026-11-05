
#include <SparkFun_TB6612.h>  // Library for controlling motors through the TB6612 driver

// =====================================================
// MOTOR PINS
// =====================================================

// Left motor connections
const int PWMA = 5;    // PWM pin controlling left motor speed
const int AIN1 = 2;    // Left motor direction control pin 1
const int AIN2 = 3;    // Left motor direction control pin 2

const int STBY = 12;   // Motor driver standby pin; HIGH enables the driver

// Right motor connections
const int BIN1 = 4;    // Right motor direction control pin 1
const int BIN2 = 7;    // Right motor direction control pin 2
const int PWMB = 6;    // PWM pin controlling right motor speed

// Create motor objects using the library.
// Arguments: direction pin 1, direction pin 2, PWM pin,
// motor direction offset, standby pin.
// Direction offset 1 means the motor uses its normal orientation.
Motor motor_left(AIN1, AIN2, PWMA, 1, STBY);
Motor motor_right(BIN1, BIN2, PWMB, 1, STBY);

// =====================================================
// SENSOR PINS
// =====================================================

const int EDGE_OUT = 8;  // Digital output from the IR edge detection sensor
const int US_TRIG = 9;   // Trigger pin of the ultrasonic distance sensor
const int US_ECHO = 10;  // Echo pin of the ultrasonic distance sensor
const int LED_PIN = 13;  // Arduino built-in LED

// =====================================================
// CONSTANTS
// =====================================================

// The IR sensor is considered to detect an edge when its output is LOW.
// Change to HIGH if your particular sensor behaves oppositely --- you can test this yourself to check. 
const int EDGE_ACTIVE_LEVEL = LOW;

// Motor speeds range from -255 to +255.
// Positive means forward; negative means backward,
// assuming both motors are wired with the expected orientation --- make sure to test this.

const int SEARCH_SPD = 150;   // Motor speed when rotating to find an opponent
const int ATTACK_SPD = 255;   // Maximum forward speed when attacking
const int REVERSE_SPD = 220;  // Backward speed during edge avoidance
const int TURN_SPD = 220;     // Rotation speed during edge avoidance

// Time durations are measured in milliseconds.
// 1000 milliseconds = 1 second.

const unsigned long REVERSE_MS = 350;      // Reverse for 0.35 seconds
const unsigned long TURN_MS = 350;         // Rotate for 0.35 seconds
const unsigned long SEARCH_FLIP_MS = 4000; // Change search direction every 4 seconds
const unsigned long ATTACK_MS = 500;       // Remain in ATTACK for 0.5 seconds
const unsigned long US_INTERVAL_MS = 60;   // Check ultrasonic distance every 60 ms

// Maximum time pulseIn() waits for the ultrasonic echo.
// Measured in microseconds, not milliseconds.
// 6000 microseconds corresponds to approximately 103 cm.
const unsigned long US_TIMEOUT_US = 6000;

// An opponent within 77 cm is considered detected.
const int ATTACK_CM = 77;

// =====================================================
// STATE MACHINE
// =====================================================

// An enum defines a set of named integer values.
// The robot can be in one of these three states.
enum State {
  SEARCH,      // Rotate to look for an opponent
  ATTACK,      // Drive forward toward an opponent
  EDGE_AVOID   // Reverse and turn away from the ring boundary
};

// Initially, the robot is in SEARCH mode.
State state = SEARCH;

// Determines which direction the robot rotates.
// +1 = one direction, -1 = the opposite direction.
int turn_dir = +1;

// Stores the time when an edge avoidance maneuver started.
unsigned long t0 = 0;

// Stores the time when the current attack started.
unsigned long attackStart = 0;

// Stores the last time the search rotation direction changed.
unsigned long lastFlip = 0;

// =====================================================
// MOTOR FUNCTIONS
// =====================================================

// Set the speeds of the two motors.
// This function returns immediately; the motors keep running
// at these speeds until another motor command is issued.
void drive(int leftSpd, int rightSpd) {

  // Command the left motor to move at leftSpd.
  motor_left.drive(leftSpd);

  // Command the right motor to move at rightSpd.
  motor_right.drive(rightSpd);
}

// Move at specified motor speeds for a fixed duration.
// This is a BLOCKING function: sensor checks cannot run
// while delay() is executing.
void driveFor(int leftSpd, int rightSpd, unsigned long duration) {

  // Start moving at the requested speeds.
  drive(leftSpd, rightSpd);

  // Keep moving for the requested number of milliseconds.
  delay(duration);

  // Stop both motors after the duration has elapsed.
  drive(0, 0);
}

// =====================================================
// EDGE SENSOR FUNCTIONS
// =====================================================

// Return true if the IR sensor currently detects an edge.
bool edgeDetected() {

  // Read the digital signal from the IR sensor.
  // Compare it against the level indicating an edge.
  // The comparison produces true or false.
  return digitalRead(EDGE_OUT) == EDGE_ACTIVE_LEVEL;
}

// =====================================================
// ULTRASONIC SENSOR FUNCTIONS
// =====================================================

// Measure the distance to the nearest object in centimeters.
// Return 999 if no valid echo is received.
int readDistanceCm() {

  // Briefly stop the motors to reduce electrical interference
  // during the ultrasonic measurement.
  drive(0, 0);

  // Wait 3 milliseconds for electrical noise to settle.
  delay(3);

  // Ensure the trigger signal starts LOW.
  digitalWrite(US_TRIG, LOW);

  // Wait 2 microseconds before producing the trigger pulse.
  delayMicroseconds(2);

  // Send HIGH to begin the ultrasonic trigger pulse.
  digitalWrite(US_TRIG, HIGH);

  // Keep the trigger HIGH for 10 microseconds.
  // The HC-SR04 requires a trigger pulse of at least 10 us.
  delayMicroseconds(10);

  // End the trigger pulse.
  digitalWrite(US_TRIG, LOW);

  // Wait for the echo pin to go HIGH and then LOW.
  // pulseIn() returns the duration of the HIGH pulse
  // in microseconds.
  // If no pulse is detected before the timeout, it returns 0.
  unsigned long dur = pulseIn(US_ECHO, HIGH, US_TIMEOUT_US);

  // If no echo was detected, return a large distance.
  // This makes opponentDetected() treat the result as no opponent.
  if (dur == 0) {
    return 999;
  }

  // Convert echo travel time into distance.
  // Speed of sound is approximately 0.0343 cm/microsecond.
  // Divide by 2 because the sound travels to the object
  // and then returns to the sensor.
  return (int)(dur * 0.0343 / 2.0);
}

// Determine whether an opponent is within attack range.
bool opponentDetected() {

  // static variables preserve their values between function calls.
  // Unlike ordinary local variables, they are initialized only once.

  // Time of the previous ultrasonic measurement.
  static unsigned long lastRead = 0;

  // Most recently measured distance, initially very large.
  static int lastCm = 999;

  // millis() returns the number of milliseconds since startup.
  // Subtracting lastRead gives the elapsed time since the last reading.
  if (millis() - lastRead >= US_INTERVAL_MS) {

    // Take a new ultrasonic measurement.
    lastCm = readDistanceCm();

    // Record when this measurement was taken.
    lastRead = millis();
  }

  // Return true if the last measured distance is within 77 cm.
  // Otherwise return false.
  return lastCm <= ATTACK_CM;
}

// =====================================================
// SETUP
// =====================================================

// setup() executes once whenever the Arduino starts or resets.
void setup() {

  // Initialize serial communication at 9600 bits per second.
  // Useful for debugging through the Serial Monitor.
  Serial.begin(9600);

  // Configure left motor control pins as outputs.
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  // Configure right motor control pins as outputs.
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  // Configure the motor driver's standby pin as an output.
  pinMode(STBY, OUTPUT);

  // Configure the edge sensor as an input.
  pinMode(EDGE_OUT, INPUT);

  // Configure ultrasonic trigger as an output.
  pinMode(US_TRIG, OUTPUT);

  // Configure ultrasonic echo as an input.
  pinMode(US_ECHO, INPUT);

  // Configure the built-in LED as an output.
  pinMode(LED_PIN, OUTPUT);

  // Ensure the ultrasonic trigger initially stays LOW.
  digitalWrite(US_TRIG, LOW);

  // Set standby HIGH to enable the motor driver.
  digitalWrite(STBY, HIGH);

  // Turn the built-in LED off.
  digitalWrite(LED_PIN, LOW);

  // Ensure both motors are stopped before starting.
  drive(0, 0);

  // Wait 5 seconds before the robot begins operating.
  // This gives you time to place it in the SUMO ring.
  delay(5000);
}

// =====================================================
// MAIN LOOP
// =====================================================

// loop() runs repeatedly after setup() finishes.
void loop() {

  // ---------------------------------------------------
  // EDGE DETECTION: HIGHEST PRIORITY
  // ---------------------------------------------------

  // Check whether an edge is detected.
  // If the robot is not already avoiding an edge,
  // immediately switch to EDGE_AVOID.
  if (edgeDetected() && state != EDGE_AVOID) {

    // Change the robot's current state.
    state = EDGE_AVOID;

    // Record the starting time of edge avoidance.
    t0 = millis();

    // Turn off the attack indicator LED.
    digitalWrite(LED_PIN, LOW);
  }

  // Execute different behavior depending on the current state.
  switch (state) {

    // =================================================
    // STATE 1: EDGE AVOIDANCE
    // =================================================
    case EDGE_AVOID: {

      // Calculate elapsed time since edge avoidance started.
      unsigned long dt = millis() - t0;

      // PHASE 1: REVERSE
      // During the first 350 ms, drive both motors backward.
      if (dt < REVERSE_MS) {

        drive(-REVERSE_SPD, -REVERSE_SPD);
      }

      // PHASE 2: TURN
      // After reversing, rotate for another 350 ms.
      else if (dt < REVERSE_MS + TURN_MS) {

        // If turn_dir is positive, rotate in one direction.
        if (turn_dir == +1) {

          // Left motor forward, right motor backward.
          drive(TURN_SPD, -TURN_SPD);
        }

        // Otherwise rotate in the opposite direction.
        else {

          // Left motor backward, right motor forward.
          drive(-TURN_SPD, TURN_SPD);
        }
      }

      // PHASE 3: FINISH
      // After reversing and turning, check the edge again.
      else {

        // If the edge is still detected, try another maneuver.
        if (edgeDetected()) {

          // Reverse the rotation direction.
          // Multiplying +1 by -1 gives -1, and vice versa.
          turn_dir *= -1;

          // Reset the maneuver timer to start reversing again.
          t0 = millis();
        }

        // If the edge is no longer detected, resume searching.
        else {

          state = SEARCH;
        }
      }

      // Exit this switch case.
      break;
    }

    // =================================================
    // STATE 2: ATTACK
    // =================================================
    case ATTACK: {

      // Turn on the built-in LED to indicate ATTACK mode.
      digitalWrite(LED_PIN, HIGH);

      // Drive both motors forward at maximum speed.
      drive(ATTACK_SPD, ATTACK_SPD);

      // Check whether 500 ms have passed since attack began.
      if (millis() - attackStart >= ATTACK_MS) {

        // Return to SEARCH so the robot can locate the opponent again.
        state = SEARCH;

        // Turn off the attack indicator LED.
        digitalWrite(LED_PIN, LOW);
      }

      // Exit the ATTACK case.
      break;
    }

    // =================================================
    // STATE 3: SEARCH
    // =================================================
    case SEARCH:
    default: {

      // Turn off the LED because the robot is not attacking.
      digitalWrite(LED_PIN, LOW);

      // Check the ultrasonic sensor for an opponent.
      if (opponentDetected()) {

        // Switch to ATTACK mode.
        state = ATTACK;

        // Record when the attack begins.
        attackStart = millis();

        // Turn on the LED.
        digitalWrite(LED_PIN, HIGH);

        // Immediately start moving forward at maximum speed.
        drive(ATTACK_SPD, ATTACK_SPD);

        // Exit SEARCH; ATTACK behavior runs on the next loop.
        break;
      }

      // If no opponent is detected, rotate to search.
      if (turn_dir == +1) {

        // Spin by moving the motors in opposite directions.
        drive(SEARCH_SPD, -SEARCH_SPD);
      }

      // Rotate in the opposite direction when turn_dir is -1.
      else {

        drive(-SEARCH_SPD, SEARCH_SPD);
      }

      // Check whether 4 seconds have passed since the last flip.
      if (millis() - lastFlip >= SEARCH_FLIP_MS) {

        // Reverse the direction of rotation.
        turn_dir *= -1;

        // Record the time of this direction change.
        lastFlip = millis();
      }

      // Exit the SEARCH case.
      break;
    }
  }
}
