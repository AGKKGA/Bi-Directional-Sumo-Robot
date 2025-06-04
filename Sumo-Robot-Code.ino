#include <math.h>

// Motor (Right)
int In1R = 12;
int In2R = 11;
int EnR = 5;

// Motor (Left)
int In1L = 13;
int In2L = 10;
int EnL = 6;

// Ultrasonic Sensors
// Front Ultrasonic
int USS_trig = 4;
int USS_echo = 2;

// Back Ultrasonic
int USS_2trig = 8;
int USS_2echo = 7;

// VARIABLES
bool direction = true; // true-right; false-left for search
bool opponent = false;
int distance_of_opponent = 70;
int counter = 0;

// New variables for bi-directional operation
enum AttackDirection { FORWARD, BACKWARD, NONE };
AttackDirection current_attack = NONE;

float getDistFront() {
  digitalWrite(USS_trig, HIGH);
  delayMicroseconds(5);
  digitalWrite(USS_trig, LOW);
  uint32_t us = pulseIn(USS_echo, HIGH);
  return (us * 0.034 / 2);
}

float getDistBack() {
  digitalWrite(USS_2trig, HIGH);
  delayMicroseconds(5);
  digitalWrite(USS_2trig, LOW);
  uint32_t us = pulseIn(USS_2echo, HIGH);
  return (us * 0.034 / 2);
}

void goForward(int percent = 100) {
  int power = ceil(2.55 * percent);
  digitalWrite(In1R, HIGH);
  digitalWrite(In2R, LOW);
  digitalWrite(In1L, HIGH);
  digitalWrite(In2L, LOW);
  analogWrite(EnR, power);
  analogWrite(EnL, power);
}

void goBackward(int percent = 100) {
  int power = ceil(2.55 * percent);
  digitalWrite(In1R, LOW);
  digitalWrite(In2R, HIGH);
  digitalWrite(In1L, LOW);
  digitalWrite(In2L, HIGH);
  analogWrite(EnR, power);
  analogWrite(EnL, power);
}

void turnRight(int percent = 100) {
  int power = ceil(2.55 * percent);
  digitalWrite(In1R, LOW);
  digitalWrite(In2R, HIGH);
  digitalWrite(In1L, HIGH);
  digitalWrite(In2L, LOW);
  analogWrite(EnR, power);
  analogWrite(EnL, power);
}

void turnLeft(int percent = 100) {
  int power = ceil(2.55 * percent);
  digitalWrite(In1R, HIGH);
  digitalWrite(In2R, LOW);
  digitalWrite(In1L, LOW);
  digitalWrite(In2L, HIGH);
  analogWrite(EnR, power);
  analogWrite(EnL, power);
}

void turn_off(int time = 100) {
  digitalWrite(In1R, LOW);
  digitalWrite(In2R, LOW);
  digitalWrite(In1L, LOW);
  digitalWrite(In2L, LOW);
  analogWrite(EnR, 0);
  analogWrite(EnL, 0);
  delay(time);
}

AttackDirection checkForOpponent() {
  float front_dist = getDistFront();
  float back_dist = getDistBack();
  
  Serial.print("Front: ");
  Serial.print(front_dist);
  Serial.print(" | Back: ");
  Serial.println(back_dist);
  
  // Check which direction has the closest opponent
  if (front_dist <= distance_of_opponent && back_dist <= distance_of_opponent) {
    // Opponent on both sides - attack the closer one
    if (front_dist <= back_dist) {
      Serial.println("Opponent front (closer)");
      return FORWARD;
    } else {
      Serial.println("Opponent back (closer)");
      return BACKWARD;
    }
  }
  else if (front_dist <= distance_of_opponent) {
    Serial.println("Opponent detected in front");
    return FORWARD;
  }
  else if (back_dist <= distance_of_opponent) {
    Serial.println("Opponent detected behind");
    return BACKWARD;
  }
  
  return NONE;
}

void attackOpponent(AttackDirection attack_dir) {
  if (attack_dir == FORWARD) {
    Serial.println("Attacking forward!");
    while (getDistFront() < distance_of_opponent) {
      goForward();
    }
  }
  else if (attack_dir == BACKWARD) {
    Serial.println("Attacking backward!");
    while (getDistBack() < distance_of_opponent) {
      goBackward();
    }
  }
  turn_off(50); // Brief stop after attack
}

void biDirectionalSearch() {
  int angle = 6;
  int speed_perc = 190;
  int turning_delay = 120;  // Increased from 40 to 120ms
  int stopping_delay = 200; // Increased from 80 to 200ms
  
  Serial.print("Search - Counter: ");
  Serial.print(counter);
  Serial.print(", Direction: ");
  Serial.print(direction ? "RIGHT" : "LEFT");
  Serial.print(", Opponent: ");
  Serial.println(opponent ? "TRUE" : "FALSE");
  
  // Check both sensors during search
  AttackDirection opponent_dir = checkForOpponent();
  
  if (opponent_dir != NONE) {
    opponent = true;
    attackOpponent(opponent_dir);
    return;
  }

  // Original search logic but now checking both sensors
  if (opponent == false || counter > 1) {
    counter = 0;
    opponent = false;
    Serial.println("=== BASIC BI-DIRECTIONAL SEARCH ===");
    
    int search_attempts = 0;
    do {
      Serial.print("Basic search attempt: ");
      Serial.println(search_attempts++);
      
      turnRight(speed_perc);
      delay(turning_delay);
      turn_off(stopping_delay);
      
      opponent_dir = checkForOpponent();
      if (opponent_dir != NONE) {
        Serial.println("Found something in basic search...");
        opponent = true;
        attackOpponent(opponent_dir);
        break;
      }
      
      // Prevent infinite loop
      if (search_attempts > 20) {
        Serial.println("Basic search timeout - switching to advanced");
        break;
      }
    } while (opponent == false);
  }
  else {
    Serial.println("=== ADVANCED BI-DIRECTIONAL SEARCH ===");
    
    if (direction == true) {
      Serial.println("Turning RIGHT and checking both sides...");
      counter += 1;
      
      for (int i = 1; i <= angle; i++) {
        Serial.print("Right turn step: ");
        Serial.println(i);
        
        turnRight(speed_perc);
        delay(turning_delay);
        turn_off(stopping_delay);
        
        opponent_dir = checkForOpponent();
        if (opponent_dir != NONE) {
          Serial.println("Found something.. attacking");
          counter = 0;
          direction = true;
          attackOpponent(opponent_dir);
          return;
        }
      }
      
      direction = false;
      Serial.println("Returning to initial position...");
      for (int i = 1; i <= angle; i++) {
        turnLeft(speed_perc);
        delay(turning_delay);
        turn_off(stopping_delay);
      }
    }
    else {
      Serial.println("Turning LEFT and checking both sides...");
      counter += 1;
      
      for (int i = 1; i <= angle; i++) {
        Serial.print("Left turn step: ");
        Serial.println(i);
        
        turnLeft(speed_perc);
        delay(turning_delay);
        turn_off(stopping_delay);
        
        opponent_dir = checkForOpponent();
        if (opponent_dir != NONE) {
          Serial.println("Found something attacking...");
          counter = 0;
          direction = false;
          attackOpponent(opponent_dir);
          return;
        }
      }
      
      direction = true;
      Serial.println("Returning to initial position...");
      for (int i = 1; i <= angle; i++) {
        turnRight(speed_perc);
        delay(turning_delay);
        turn_off(stopping_delay);
      }
    }
  }
  
  // Reset counter if it gets too high to prevent getting stuck
  if (counter > 3) {
    Serial.println("Counter reset - preventing infinite loops");
    counter = 0;
    opponent = false;
    direction = !direction; // Switch direction
  }
}

void setup() {
  Serial.begin(9600);
  
  // Ultrasonic sensors
  pinMode(USS_trig, OUTPUT);
  pinMode(USS_echo, INPUT);
  pinMode(USS_2trig, OUTPUT);
  pinMode(USS_2echo, INPUT);
  
  // Motors
  pinMode(In1R, OUTPUT);
  pinMode(In2R, OUTPUT);
  pinMode(EnR, OUTPUT);
  pinMode(In1L, OUTPUT);
  pinMode(In2L, OUTPUT);
  pinMode(EnL, OUTPUT);
  
  // Stop motors initially
  analogWrite(EnR, 0);
  analogWrite(EnL, 0);
  
  digitalWrite(13, HIGH); // Ready LED
  
  // 5 second countdown
  Serial.println("SUMO ROBOT STARTING IN:");
  for (int i = 5; i > 0; i--) {
    Serial.print(i);
    Serial.println(" seconds...");
    delay(1000);
  }
  
  Serial.println("GO! FIGHT!");
  digitalWrite(13, LOW); // Turn off ready LED
  
  Serial.print("Initial front distance: ");
  Serial.println(getDistFront());
  Serial.print("Initial back distance: ");
  Serial.println(getDistBack());
}

void loop() {
  Serial.println("=== MAIN LOOP START ===");
  
  // Check both directions for immediate threats
  AttackDirection opponent_dir = checkForOpponent();
  
  if (opponent_dir != NONE) {
    Serial.println("Found opponent from main loop - attacking!");
    attackOpponent(opponent_dir);
    // Reset variables after attack
    opponent = false;
    counter = 0;
  }
  else {
    Serial.println("No immediate threat - starting bi-directional search");
    biDirectionalSearch();
  }
  
  // Small delay to prevent overwhelming the serial monitor
  delay(50);
}
