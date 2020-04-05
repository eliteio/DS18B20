#include <DS18B20.h>

const int MAXRETRY = 3;
const uint32_t msSampleTime = 2500;
const uint32_t msPublishTime = 30000;
const int16_t pinLED = D7;

const int16_t dsVCC  = D2;
const int16_t dsData = D3;
const int16_t dsGND  = D4;

DS18B20 ds18b20(dsData);

const int nSENSORS = 2;
float celsius[nSENSORS] = {NAN, NAN};
retained uint8_t sensorAddresses[nSENSORS][8];

void setup() {
  pinMode(pinLED, OUTPUT);

  pinMode(dsGND, OUTPUT);
  digitalWrite(dsGND, LOW);
  pinMode(dsVCC, OUTPUT);
  digitalWrite(dsVCC, HIGH);

  delay(1000);

  ds18b20.resetsearch();                 // initialise for sensor search
  for (int i = 0; i < nSENSORS; i++) {   // try to read the sensor addresses
    ds18b20.search(sensorAddresses[i]); // and if available store
  }
}

void loop() {
  static uint32_t msSample = 0;
  static uint32_t msPublish = 0;

  if (millis() - msSample >= msSampleTime) {
    msSample = millis();
    for (int i = 0; i < nSENSORS; i++) {
      float temp = getTemp(sensorAddresses[i]);
      if (!isnan(temp)) celsius[i] = temp;
    }
  }

  if (millis() - msPublish >= msPublishTime) {
    msPublish = millis();
    Serial.println("Publishing now.");
    publishData();
  }
}

double getTemp(uint8_t addr[8]) {
  double _temp;
  int   i = 0;

  do {
    _temp = ds18b20.getTemperature(addr);
  } while (!ds18b20.crcCheck() && MAXRETRY > i++);

  if (i < MAXRETRY) {
    //_temp = ds18b20.convertToFahrenheit(_temp);
    Serial.println(_temp);
  }
  else {
    _temp = NAN;
    Serial.println("Invalid reading");
  }

  return _temp;
}

void publishData() {
  char szInfo[64];
  snprintf(szInfo, sizeof(szInfo), "%.1f °C, %.1f °C", celsius[0], celsius[1]);
  Particle.publish("dsTmp", szInfo, PRIVATE);
}
