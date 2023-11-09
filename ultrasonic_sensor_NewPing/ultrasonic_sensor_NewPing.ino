#include "NewPing.h"
#define TRIGGER_PIN D5
#define ECHO_PIN D6
#define MAX_DISTANCE 400	
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
void setup() {
	Serial.begin(9600);
}
void loop() {
	Serial.print("Distance = ");
	Serial.print(sonar.ping_cm());
	Serial.println(" cm");
	delay(500);
}
