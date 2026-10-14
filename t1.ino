
/*
  UniKo DVI v3.2 - Traffic Swerving Robot
  Controller: ATmega32A

  Motors: 12, 13, 14, 15
  Ultrasonic trigger: D0
  Ultrasonic echo: D1

  Behaviour:
  1. Drive forward.
  2. Detect traffic ahead.
  3. Scan left, then scan right.
  4. Prefer the left if clear; otherwise use the right.
  5. Reverse and stop if neither side appears clear.
  6. Stop after 20 seconds to simulate road crack repair.
  7. Resume after the repair pause.
*/

#include <Arduino.h>

// ==================== MOTOR PINS ====================

const byte LEFT_FORWARD_PIN  = 12;
const byte LEFT_BACKWARD_PIN = 13;
const byte RIGHT_FORWARD_PIN = 14;
const byte RIGHT_BACKWARD_PIN = 15;

// ==================== ULTRASONIC SENSOR ====================

const byte TRIG_PIN = 0;
const byte ECHO_PIN = 1;

// ==================== SETTINGS ====================

const int OBSTACLE_DISTANCE = 20;

// Smaller values mean shorter turns
const unsigned int SCAN_TURN_TIME = 120;
const unsigned int AVOID_TURN_TIME = 200;
const unsigned int PASS_TIME = 600;
const unsigned int BACKUP_TIME = 350;

const unsigned long ROAD_TIME = 20000UL;
const unsigned long REPAIR_TIME = 6000UL;

// ==================== TIMER VARIABLES ====================

unsigned long roadStartTime = 0;
unsigned long repairStartTime = 0;

bool repairing = false;
bool repairCompleted = false;

// ==================== FUNCTION DECLARATIONS ====================

void setup();
void loop();

void setupMotors();
long readDistance();

void stopMotors();
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();

void avoidObstacle();
void scanBothSides();
void passOnLeft();
void passOnRight();
void repairRoadCrack();

// ==================== SETUP ====================

void setup() {
  setupMotors();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  stopMotors();

  roadStartTime = millis();
}

// ==================== MAIN LOOP ====================

void loop() {
  unsigned long now = millis();

  // Stop once to demonstrate road crack repair
  if (!repairing && !repairCompleted &&
      now - roadStartTime >= ROAD_TIME) {
    repairRoadCrack();
    return;
  }

  // Keep stopped during repair
  if (repairing) {
    stopMotors();

    if (now - repairStartTime >= REPAIR_TIME) {
      repairing = false;
      repairCompleted = true;
    }

    return;
  }

  long distance = readDistance();

  if (distance > 0 && distance <= OBSTACLE_DISTANCE) {
    avoidObstacle();
  } else if (distance == 999) {
    // No echo received; treat as no detected obstacle
    moveForward();
  } else {
    moveForward();
  }

  delay(40);
}

// ==================== MOTOR SETUP ====================

void setupMotors() {
  pinMode(LEFT_FORWARD_PIN, OUTPUT);
  pinMode(LEFT_BACKWARD_PIN, OUTPUT);

  pinMode(RIGHT_FORWARD_PIN, OUTPUT);
  pinMode(RIGHT_BACKWARD_PIN, OUTPUT);
}

// ==================== ULTRASONIC DISTANCE ====================

long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
      pulseIn(ECHO_PIN, HIGH, 25000UL);

  if (duration == 0) {
    return 999;
  }

  return duration * 0.0343 / 2.0;
}

// ==================== STOP ====================

void stopMotors() {
  digitalWrite(LEFT_FORWARD_PIN, LOW);
  digitalWrite(LEFT_BACKWARD_PIN, LOW);

  digitalWrite(RIGHT_FORWARD_PIN, LOW);
  digitalWrite(RIGHT_BACKWARD_PIN, LOW);
}

// ==================== FORWARD ====================

void moveForward() {
  // Corrected for the observed left motor direction
  digitalWrite(LEFT_FORWARD_PIN, LOW);
  digitalWrite(LEFT_BACKWARD_PIN, HIGH);

  digitalWrite(RIGHT_FORWARD_PIN, HIGH);
  digitalWrite(RIGHT_BACKWARD_PIN, LOW);
}

// ==================== BACKWARD ====================

void moveBackward() {
  digitalWrite(LEFT_FORWARD_PIN, HIGH);
  digitalWrite(LEFT_BACKWARD_PIN, LOW);

  digitalWrite(RIGHT_FORWARD_PIN, LOW);
  digitalWrite(RIGHT_BACKWARD_PIN, HIGH);
}

// ==================== TURN LEFT ====================

void turnLeft() {
  digitalWrite(LEFT_FORWARD_PIN, HIGH);
  digitalWrite(LEFT_BACKWARD_PIN, LOW);

  digitalWrite(RIGHT_FORWARD_PIN, HIGH);
  digitalWrite(RIGHT_BACKWARD_PIN, LOW);
}

// ==================== TURN RIGHT ====================

void turnRight() {
  digitalWrite(LEFT_FORWARD_PIN, LOW);
  digitalWrite(LEFT_BACKWARD_PIN, HIGH);

  digitalWrite(RIGHT_FORWARD_PIN, LOW);
  digitalWrite(RIGHT_BACKWARD_PIN, HIGH);
}

// ==================== OBSTACLE DETECTED ====================

void avoidObstacle() {
  stopMotors();
  delay(150);

  scanBothSides();
}

// ==================== SCAN LEFT AND RIGHT ====================

void scanBothSides() {
  // Look left first
  turnLeft();
  delay(SCAN_TURN_TIME);

  stopMotors();
  delay(100);

  long leftDistance = readDistance();

  // Rotate across the centre to look right
  turnRight();
  delay(SCAN_TURN_TIME * 2);

  stopMotors();
  delay(100);

  long rightDistance = readDistance();

  // Return to the original forward heading
  turnLeft();
  delay(SCAN_TURN_TIME);

  stopMotors();
  delay(150);

  bool leftClear = leftDistance > OBSTACLE_DISTANCE;
  bool rightClear = rightDistance > OBSTACLE_DISTANCE;

  // Prefer left when both sides look clear
  if (leftClear) {
    passOnLeft();
  }
  else if (rightClear) {
    passOnRight();
  }
  else {
    // Both sides appear blocked
    moveBackward();
    delay(BACKUP_TIME);

    stopMotors();
    delay(500);
  }
}

// ==================== PASS ON LEFT ====================

void passOnLeft() {
  turnLeft();
  delay(AVOID_TURN_TIME);

  stopMotors();
  delay(100);

  moveForward();
  delay(PASS_TIME);

  turnRight();
  delay(AVOID_TURN_TIME);

  stopMotors();
  delay(100);
}

// ==================== PASS ON RIGHT ====================

void passOnRight() {
  turnRight();
  delay(AVOID_TURN_TIME);

  stopMotors();
  delay(100);

  moveForward();
  delay(PASS_TIME);

  turnLeft();
  delay(AVOID_TURN_TIME);

  stopMotors();
  delay(100);
}

// ==================== ROAD CRACK REPAIR ====================

void repairRoadCrack() {
  stopMotors();

  repairing = true;
  repairStartTime = millis();
}
