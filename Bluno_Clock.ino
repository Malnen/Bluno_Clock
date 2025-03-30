int rPin = 11;
int gPin = 10;
int bPin = 9;

unsigned long lastActivityTime = 0;

int pulseValue = 0;
int pulseDirection = 1;

void setup() {
  Serial.begin(115200);
  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    processRGBData(data);
    lastActivityTime = millis();
    sendCompletionSignal();
  }

  if (millis() - lastActivityTime >= 10000) {
    slowPulse();
    delay(25);
  }
}


void processRGBData(String data) {
  data.trim();
  if (data.startsWith("[") && data.endsWith("]")) {
    data = data.substring(1, data.length() - 1);
    int firstComma = data.indexOf(',');
    int secondComma = data.lastIndexOf(',');

    int r = data.substring(0, firstComma).toInt();
    int g = data.substring(firstComma + 1, secondComma).toInt();
    int b = data.substring(secondComma + 1).toInt();

    if (r > -1 && g > -1 && b > -1) {
      analogWrite(rPin, 255 - r);
      analogWrite(gPin, 255 - g);
      analogWrite(bPin, 255 - b);

      lastActivityTime = millis();
    }
  }
}

void slowPulse() {
  pulseValue += pulseDirection * 5;
  if (pulseValue <= 0 || pulseValue >= 255) {
    pulseDirection = -pulseDirection;
  }

  analogWrite(rPin, pulseValue);
  analogWrite(gPin, pulseValue);
  analogWrite(bPin, pulseValue);
}

void sendCompletionSignal() {
  Serial.println("DONE");
  Serial.flush();
  while (Serial.available() > 0) {
    Serial.read();
  }
}