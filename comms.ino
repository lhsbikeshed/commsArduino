#define PIN_LEDS  9
#define PIN_DISKSENSE  6
#define PIN_ALED  5
#define PIN_DISKCORRECT  4

boolean powerOn = false;

//disk
long readTimer = 0;
byte lastDiskSense = 0;
boolean activityLight = false;
boolean readingDisk = false;

//damage
long damageTimer = 0;
long nextFlicker = 100;
long lastFlicker = 0;

boolean lightState = false;

//serial receiving
char buffer[10];
byte bufPtr = 0;

void setup() {
  pinMode(PIN_DISKSENSE, INPUT);
  digitalWrite(PIN_DISKSENSE, HIGH);
  pinMode(PIN_DISKCORRECT, INPUT);
  digitalWrite(PIN_DISKCORRECT, HIGH);

  pinMode(PIN_LEDS, OUTPUT);
  pinMode(PIN_ALED, OUTPUT);
  Serial.begin(9600);
}

void processSerial(char* c) {
  if (c[0] == 'p') {    //power off
    powerOn = false;
  } else if (c[0] == 'P') {
    powerOn = true;
  } else if (c[0] == 'd') {
    damageTimer = millis();
    lastFlicker = millis();
    nextFlicker = 80;
  }
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == ',') {
      bufPtr = 0;
      processSerial(buffer);
    } else {
      if (bufPtr + 1 < 10) {
        buffer[bufPtr] = c;
        bufPtr++;
      } else {
        bufPtr = 0;
      }
    }
  }

  byte diskSense = digitalRead(PIN_DISKSENSE);

  if (diskSense != lastDiskSense) {
    if (diskSense == 0) {
      //disk insert
      readTimer = millis();
      readingDisk = true;
      Serial.print("D,");
    } else {
      Serial.print("d,");
    }
    lastDiskSense = diskSense;
  }

  //everything past this is power dependant
  if (powerOn == false) {
    digitalWrite(PIN_LEDS, LOW);

    return;
  }

  if (readTimer + 5500 > millis()) {
    if (random(100) < 5) {
      activityLight = !activityLight;
    }
  } else {
    if (readingDisk) {
      readingDisk = false;
      if (digitalRead(PIN_DISKCORRECT)) {
        Serial.print("I,");
      } else {
        Serial.print("i,");
      }
    }
    activityLight = false;
  }
  digitalWrite(PIN_ALED, activityLight);

  if (damageTimer + 2000 > millis()) {
    if (lastFlicker + nextFlicker < millis()) {
      nextFlicker = 50 + random(50);
      lastFlicker = millis();
      lightState = !lightState;
    }
  } else {
    lightState = powerOn;
  }
  digitalWrite(PIN_LEDS, lightState == true ? HIGH : LOW);
}
