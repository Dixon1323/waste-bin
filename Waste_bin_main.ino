#include <NewPing.h>
#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial sim800l(6, 7);

#define WServoPin 11
#define W_sound 8

Servo WServo;

const int TRIGGER_PIN_1 = 2;
const int ECHO_PIN_1 = 3;
const int TRIGGER_PIN_2 = 4;
const int ECHO_PIN_2 = 5;
const int MAX_DISTANCE = 100;
unsigned int obst_dist;
unsigned int waste_dist;
const int wasteThreshold = 26;
const int wasteConfirmationDuration = 20000;
bool isWasteFull = false;
unsigned long wasteFullStartTime = 0;
unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;
const long interval = 300000;  // delay for sending sms
const long interval1 = 5000;   // delay for checking signal
char phoneNumber[] = "Your phone no without country code";
char smsMessage[] = "Message to be send";
int count = 0;

NewPing sonar1(TRIGGER_PIN_1, ECHO_PIN_1, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN_2, ECHO_PIN_2, MAX_DISTANCE);


void ultrasonic_sensor() {
  obst_dist = sonar1.ping_cm();
  waste_dist = sonar2.ping_cm();
  //umcomment below codes if you want to measure values of both ultrasonic sensors
  // Serial.print("obtacle distance: ");
  // Serial.print(obst_dist);
  // Serial.print(" cm\t");
  // Serial.print("waste Distance: ");
  // Serial.print(waste_dist);
  // Serial.println(" cm");
  delay(200);
}

void rotateClockwise() {
  WServo.attach(WServoPin, 500, 2400);
  for (int i = WServo.read(); i <= 140; i++) {
    WServo.write(i);
    //Serial.println ("clockwise");
    delay(15);
  }
}

void rotateCounterclockwise() {
  WServo.attach(WServoPin, 500, 2400);
  for (int i = WServo.read(); i >= 0; i--) {
    WServo.write(i);
    //Serial.println ("anticlockw");
    delay(15);
  }
}
void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  pinMode(9, OUTPUT); //gsm no signal LED
  pinMode(10, OUTPUT); //Waste full LED
  digitalWrite(10, LOW);
  digitalWrite(9, HIGH);
  WServo.write(0);
  delay(100);
  sim800l.println("AT");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  sim800l.println("AT+CMGF=1");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  gsm_signal();
  WServo.attach(WServoPin, 500, 2400);
}

void loop() {

  ultrasonic_sensor();
  unsigned long currentMillis1 = millis();
  if (currentMillis1 - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis1;
    gsm_signal();
  }
  if (isWasteFull) {
    unsigned long currentMillis = millis();
    if (currentMillis - wasteFullStartTime >= wasteConfirmationDuration) {
      isWasteFull = false;
      WServo.attach(WServoPin, 500, 2400);
    }
  } else {
    if (waste_dist < wasteThreshold && waste_dist >= 0) {
      if (!isWasteFull) {
        isWasteFull = true;
        wasteFullStartTime = millis();
        WServo.detach();
      }
    } else {
      isWasteFull = false;
    }
  }

  if (obst_dist <= 50 && obst_dist >= 1 && !isWasteFull) { //person detecting distance
    digitalWrite(W_sound, HIGH);
    delay(100);
    digitalWrite(W_sound, LOW);
    rotateClockwise();
    delay(5000);//time for which lid stays open
  } else {
    digitalWrite(W_sound, LOW);
    rotateCounterclockwise();
  }

  if (isWasteFull) {
    digitalWrite(10, HIGH);
    count++;
    WServo.detach();
    unsigned long currentMillis = millis();
          if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      Serial.println("send sms from loop");
      send_sms();
    }
  } else {
    digitalWrite(10, LOW);
    count = 0;
    Serial.println(waste_dist);
  }
}


void send_sms() {
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phoneNumber);
  sim800l.println("\"");
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }

  sim800l.print(smsMessage);
  sim800l.write(26);
  delay(1000);
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  Serial.println("Send SMS from function");
}

void gsm_signal() { //check for gsm signal periodically
  WServo.detach();
  sim800l.println("AT+CREG?");
  delay(1000);
  while (sim800l.available()) {
    char response = sim800l.read();
    Serial.write(response);
    if (response == '+') {
      if (sim800l.find("+CREG: 0,1")) {
        digitalWrite(9, LOW);

      } else {
        digitalWrite(9, HIGH);
      }
    }
  }
}