#include "DS18B20.h"

DS18B20::DS18B20(uint16_t pin, bool singleDrop)
  : _singleDrop(singleDrop)
{
  ds = new OneWire(pin);
}

boolean DS18B20::search()
{
  boolean isSuccess = ds->search(_addr);

  return isSuccess;
}

boolean DS18B20::search(uint8_t addr[8])
{

  boolean isSuccess = ds->search(_addr);
  if (isSuccess) memcpy(addr, _addr, 8);
  else           memset(addr, 0, 8);
  return isSuccess;
}

DS18B20::~DS18B20()
{
  delete(ds);
}

void DS18B20::setAddress(uint8_t addr[8])
{
  memcpy(_addr, addr, 8);
}

void DS18B20::resetsearch()
{
  ds->reset_search();
}

void DS18B20::setResolution(uint8_t newResolution)
{
  if (_singleDrop && !_addr[0])
  {
    resetsearch();
    search();
  }
  setResolution(_addr, newResolution);
}

void DS18B20::setResolution(uint8_t addr[0], uint8_t newResolution)
{
  if (!addr[0]) return;

  ds->reset();
  ds->select(addr);

  switch (newResolution)
  {
    case 12:
      ds->write(TEMP_12_BIT);
      break;
    case 11:
      ds->write(TEMP_11_BIT);
      break;
    case 10:
      ds->write(TEMP_10_BIT);
      break;
    case 9:
    default:
      ds->write(TEMP_9_BIT);
      break;
  }
  HAL_Delay_Milliseconds(20);
  ds->reset();
}

bool DS18B20::readPowerSupply()
{
  if (_singleDrop && !_addr[0])
  {
    resetsearch();
    search();
  }
  return readPowerSupply(_addr);
}

bool DS18B20::readPowerSupply(uint8_t addr[8])
{
  if (!addr[0]) return false;

  ds->reset();
  ds->select(addr);

  bool ret = false;
  ds->write(READPOWERSUPPLY);
  if (ds->read_bit() == 0) ret = true;
  ds->reset();
  return ret;
}

void DS18B20::getROM(char szROM[])
{
  if (_singleDrop && !_addr[0])
  {
    resetsearch();
    search();
  }

  sprintf(szROM, "%02X %02X %02X %02X %02X %02X %02X %02X", _addr[0], _addr[1], _addr[2], _addr[3], _addr[4], _addr[5], _addr[6], _addr[7]);
}

byte DS18B20::getChipType()
{
  if (_singleDrop && !_addr[0])
  {
    resetsearch();
    search();
  }
  return getChipType(_addr);
}

byte DS18B20::getChipType(uint8_t addr[8])
{
  return addr[0];
}

char* DS18B20::getChipName()
{
  if (_singleDrop && !_addr[0])
  {
    resetsearch();
    search();
  }
  return getChipName(_addr);
}

char* DS18B20::getChipName(uint8_t addr[8])
{
  switch (addr[0])
  {
    case 0x10: return "DS18S20"; break;
    case 0x28: return "DS18B20"; break;
    case 0x22: return "DS1822" ; break;
    default:   return "Unknown"; break;
  }

  return "";
}

float DS18B20::getTemperature(bool forceSelect)
{
  if (_singleDrop && !_addr[0])
  {
    resetsearch();
    search();
  }
  return getTemperature(forceSelect ? _addr : NULL);
}

float DS18B20::getTemperature(uint8_t addr[8])
{
  bool forceSelect = (addr != NULL);
  if (!_singleDrop && !addr[0]) return NAN;

  ds->reset();
  if (_singleDrop && !forceSelect)
    ds->skip();
  else
    ds->select(addr);
  ds->write(0x44);        // start conversion, with parasite power on at the end
  delay(750);     // maybe 750ms is enough, maybe not
                  // we might do a ds.depower() here, but the reset will take care of it.
  ds->reset();
  if (_singleDrop && !forceSelect)
    ds->skip();
  else
    ds->select(addr);
  ds->write(0xBE);         // Read Scratchpad

  for (int i = 0; i < 9; i++)
  {           // we need 9 bytes
    _data[i] = ds->read();
    //Serial.println("DS Read Data:");
    //Serial.println(data[i], HEX);
    //Serial.println(" ");
  }
  _dataCRC = (OneWire::crc8(_data, 8));
  //Serial.println(_dataCRC, HEX);
  _readCRC = (_data[8]);
  //Serial.println(_readCRC, HEX);

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (_data[1] << 8) | _data[0];

  if (addr[0] == 0x10)
  {
    raw = raw << 3; // 9 bit resolution default
    if (_data[7] == 0x10)
    {
// "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - _data[6];
    }
  }
  else
  {
    byte cfg = (_data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
                                          //// default is 12 bit resolution, 750 ms conversion time
  }
  return (float)raw / 16.0;
}

float DS18B20::convertToFahrenheit(float celsius)
{
  return celsius * 1.8 + 32.0;
}

bool DS18B20::crcCheck()
{
  if (_dataCRC != _readCRC)
  {
    Serial.println("CRC Failed");
    return false;
  }

  return true;
}
