#pragma once

#include <Particle.h>
#include <OneWire.h>
#include <math.h>

const int MAX_NAME    = 8;
const int MAX_RETRIES = 3;

// known sensor types
enum DS18Type  : uint8_t
{
  WIRE_UNKNOWN = 0x00,
  WIRE_DS1820  = 0x10,
  WIRE_DS1822  = 0x22,
  WIRE_DS2438  = 0x26,
  WIRE_DS18B20 = 0x28,
};

// parasite powered or not
const uint8_t READPOWERSUPPLY = 0xB4;

// device resolution
enum DS18res   : uint8_t
{
  TEMP_9_BIT   = 0x1F, //  9 bit
  TEMP_10_BIT  = 0x3F, // 10 bit
  TEMP_11_BIT  = 0x5F, // 11 bit
  TEMP_12_BIT  = 0x7F, // 12 bit
};

class DS18B20 {
private:
  OneWire*     ds;
  byte         _data[12];
  byte         _addr[8];
  byte         _dataCRC;
  byte         _readCRC;
  bool         _singleDrop;

public:
  DS18B20(uint16_t pi, bool singleDrop = false);
  ~DS18B20();
  boolean      search();
  boolean      search(uint8_t addr[8]);
  void         setAddress(uint8_t addr[8]);
  void         resetsearch();
  bool         setResolution(uint8_t newResolution);
  bool         setResolution(uint8_t addr[8], uint8_t newResolution);
  bool         readPowerSupply();
  bool         readPowerSupply(uint8_t addr[8]);
  bool         getROM(char szROM[]);
  byte         getChipType();
  byte         getChipType(uint8_t addr[8]);
  const char*  getChipName();
  const char*  getChipName(uint8_t addr[8]);
  float        getTemperature(bool forceSelect = false);
  float        getTemperature(uint8_t addr[8], bool forceSelect = false);
  float        convertToFahrenheit(float celsius);
  bool         crcCheck();
};
