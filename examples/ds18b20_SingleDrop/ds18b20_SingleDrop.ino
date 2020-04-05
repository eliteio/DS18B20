#include <DS18B20.h>
#include <math.h>

const int      MAXRETRY          = 4;
const uint32_t msSAMPLE_INTERVAL = 2500;
const uint32_t msMETRIC_PUBLISH  = 10000;

const int16_t dsVCC  = D2;
const int16_t dsData = D3;
const int16_t dsGND  = D4;

// Sets Pin D3 as data pin and the only sensor on bus
DS18B20  ds18b20(dsData, true); 

char     szInfo[64];
double   celsius;
double   fahrenheit;
uint32_t msLastMetric;
uint32_t msLastSample;

void setup() {
  Serial.begin(115200);
  
  pinMode(dsGND, OUTPUT);
  digitalWrite(dsGND, LOW);
  pinMode(dsVCC, OUTPUT);
  digitalWrite(dsVCC, HIGH);

  delay(1000);
}

void loop() {
  if (millis() - msLastSample >= msSAMPLE_INTERVAL){
    getTemp();
  }

  if (millis() - msLastMetric >= msMETRIC_PUBLISH){
    Serial.println("Publishing now.");
    publishData();
  }
}

void publishData(){
  sprintf(szInfo, "%2.2f", fahrenheit);
  Particle.publish("dsTmp", szInfo, PRIVATE);
  msLastMetric = millis();
}

void getTemp(){
  float _temp;
  int   i = 0;

  do {
    _temp = ds18b20.getTemperature();
  } while (!ds18b20.crcCheck() && MAXRETRY > i++);

  if (i < MAXRETRY) {
    celsius = _temp;
    fahrenheit = ds18b20.convertToFahrenheit(_temp);
    Serial.println(fahrenheit);
  }
  else {
    celsius = fahrenheit = NAN;
    Serial.println("Invalid reading");
  }
  msLastSample = millis();
}
