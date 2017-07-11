#pragma once

#include <Particle.h>
#include <OneWire.h>
#include <math.h>

#define MAX_NAME 8
#define MAX_RETRIES 3

// Device resolution
#define TEMP_9_BIT  0x1F //  9 bit
#define TEMP_10_BIT 0x3F // 10 bit
#define TEMP_11_BIT 0x5F // 11 bit
#define TEMP_12_BIT 0x7F // 12 bit
//Parasite Powered or Not
#define READPOWERSUPPLY 0xB4

class DS18B20 {
private:
  OneWire* ds;
  byte _data[12];
  byte _addr[8];
  byte _dataCRC;
  byte _readCRC;
  bool _singleDrop;

public:
  DS18B20(uint16_t pi, bool singleDrop = false);
  ~DS18B20();
  boolean search();
  boolean search(uint8_t addr[8]);
  void setAddress(uint8_t addr[8]);
  void resetsearch();
  void setResolution(uint8_t newResolution);
  void setResolution(uint8_t addr[8], uint8_t newResolution);
  bool readPowerSupply();
  bool readPowerSupply(uint8_t addr[8]);
  void getROM(char szROM[]);
  byte getChipType();
  byte getChipType(uint8_t addr[8]);
  char* getChipName();
  char* getChipName(uint8_t addr[8]);
  float getTemperature(bool forceSelect = false);
  float getTemperature(uint8_t addr[8]);
  float convertToFahrenheit(float celsius);
  bool crcCheck();
};
