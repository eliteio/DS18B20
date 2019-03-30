#include "DS18B20.h"

DS18B20::DS18B20(uint16_t pin, bool singleDrop)
{
  memset(_data, 0, sizeof(_data));
  memset(_addr, 0, sizeof(_addr));
  _dataCRC    = 0; 
  _readCRC    = 0;
  _singleDrop = singleDrop;
  ds          = new OneWire(pin);
}

boolean DS18B20::search()
{
  return search(_addr);
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

bool DS18B20::setResolution(uint8_t newResolution)
{
  if (_singleDrop && _addr[0] == WIRE_UNKNOWN)
  {
    resetsearch();
    if (!search()) return false;
  }
  return setResolution(_addr, newResolution);
}

bool DS18B20::setResolution(uint8_t addr[8], uint8_t newResolution)
{
  if (addr[0] == WIRE_UNKNOWN) return false;

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
  return true;
}

bool DS18B20::readPowerSupply()
{
  if (_singleDrop && _addr[0] == WIRE_UNKNOWN)
  {
    resetsearch();
    if (!search()) return false;
  }
  return readPowerSupply(_addr);
}

bool DS18B20::readPowerSupply(uint8_t addr[8])
{
  if (addr[0] == WIRE_UNKNOWN) return false;

  ds->reset();
  ds->select(addr);

  bool ret = false;
  ds->write(READPOWERSUPPLY);
  if (ds->read_bit() == 0) ret = true;
  ds->reset();
  return ret;
}

bool DS18B20::getROM(char szROM[])
{
  if (_singleDrop && _addr[0] == WIRE_UNKNOWN)
  {
    resetsearch();
    if (!search()) return false;
  }

  sprintf(szROM, "%02X %02X %02X %02X %02X %02X %02X %02X", _addr[0], _addr[1], _addr[2], _addr[3], _addr[4], _addr[5], _addr[6], _addr[7]);
  return true;
}

byte DS18B20::getChipType()
{
  if (_singleDrop && _addr[0] == WIRE_UNKNOWN)
  {
    resetsearch();
    if (!search()) return WIRE_UNKNOWN;
  }
  return getChipType(_addr);
}

byte DS18B20::getChipType(uint8_t addr[8])
{
  return addr[0];
}

const char* DS18B20::getChipName()
{
  if (_singleDrop && _addr[0] == WIRE_UNKNOWN)
  {
    resetsearch();
    if (!search()) return "not found";
  }
  return getChipName(_addr);
}

const char* DS18B20::getChipName(uint8_t addr[8])
{
  switch (addr[0])
  {
    case 0x10: return "DS18S20"; break;
    case 0x28: return "DS18B20"; break;
    case 0x22: return "DS1822" ; break;
    case 0x26: return "DS2438" ; break;
    default:   return "Unknown"; break;
  }

  return "impossible";
}

float DS18B20::getTemperature(bool forceSelect)
{
  if (_singleDrop && _addr[0] == WIRE_UNKNOWN)
  {
    resetsearch();
    if (!search()) return NAN;
  }
  return getTemperature(_addr, forceSelect);
}

float DS18B20::getTemperature(uint8_t addr[8], bool forceSelect)
{
  float celsius = NAN;
  if (!_singleDrop && addr[0] == WIRE_UNKNOWN) return NAN;

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
  if (addr[0] == WIRE_DS2438) {
    ds->write(0x00,0);     // DS2438 requires a page to read
  }

  for (int i = 0; i < 9; i++)
  {           // we need 9 bytes
    _data[i] = ds->read();
  }
  _dataCRC = (OneWire::crc8(_data, 8));
  _readCRC = (_data[8]);
  if (_dataCRC != _readCRC) return NAN;

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.

  int16_t raw;
  switch(addr[0]) 
  { // sensor type 
    case WIRE_DS1820:
      raw = ((_data[1] << 8) | _data[0]) << 3;          // 9 bit resolution default
      if (_data[7] == 0x10)                             // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - _data[6];     
      celsius = (float)raw / 16.0;
      break;

    case WIRE_DS1822:
    case WIRE_DS18B20:
      switch (_data[4] & 0x60)
      { // lower resolution means shorter conversion time, low bits need masking
        case 0x00: _data[0] &= ~0b111; break;           //  9 bit  93.75 ms
        case 0x20: _data[0] &= ~0b011; break;           // 10 bit 187.50 ms
        case 0x40: _data[0] &= ~0b001; break;           // 11 bit 375.00 ms
        default: break;                                 // 12 bit 750.00 ms 
      }
      celsius = (int16_t)((_data[1] << 8) | _data[0]) / 16.0;
      break;
      
    case WIRE_DS2438:
      celsius = (int16_t)_data[2] + ((float)(_data[1] >> 3) / 32.0) * (_data[2] & 0x80 ? -1.0 : 1.0);           
      break;
      
    default:
      celsius = NAN;
      break;                            
  }
  
  return celsius;
}

float DS18B20::convertToFahrenheit(float celsius)
{
  return celsius * 1.8 + 32.0;
}

bool DS18B20::crcCheck()
{
  return (_dataCRC == _readCRC);
}
