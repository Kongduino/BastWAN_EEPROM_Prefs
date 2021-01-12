#undef min
#undef max
#include <string>
#include <Wire.h>

using namespace std;
template class basic_string<char>; // https://github.com/esp8266/Arduino/issues/1136
// Required or the code won't compile!
namespace std _GLIBCXX_VISIBILITY(default) {
_GLIBCXX_BEGIN_NAMESPACE_VERSION
void __throw_bad_alloc() {}
}

#include "SparkFun_External_EEPROM.h"
// Click here to get the library: http://librarymanager/All#SparkFun_External_EEPROM
#include "ArduinoJson.h"
// Click here to get the library: http://librarymanager/All#ArduinoJson
ExternalEEPROM myMem;
unsigned char buf[97];
double myFreq = 868125000;
uint8_t mySF = 10, myBW = 8;
string deviceName = "My BastWAN";

void setup() {
  SerialUSB.begin(115200);
  SerialUSB.flush();
  delay(3000);
  Serial.println("\nEEPROM Prefs Example");
  Serial.println("Buffer size: " + String(SERIAL_BUFFER_SIZE));
  Wire.begin(SDA, SCL);
  Wire.setClock(100000);
  // bool begin(uint8_t deviceAddress = 0b1010000, TwoWire &wirePort = Wire);
  // deviceAddress = 0x50
  // flip switches on the breakout board to change it to between 0x51 and 0x57.
  if (myMem.begin() == false) {
    Serial.println("No memory detected. Freezing.");
    while (1)
      ;
  }
  // To erase the prefs:
  // memset(buf, 0, 97);
  // myMem.write(0, buf, 32);
  // myMem.write(32, buf + 32, 32);
  // myMem.write(64, buf + 64, 32);
  memset(buf, 0, 97);
  myMem.read(0, buf, 32);
  myMem.read(32, buf + 32, 32);
  myMem.read(64, buf + 64, 32);
  // Let's limit the JSON string size to 96 for now.
  hexDump(96);
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, buf);
  if (error) {
    SerialUSB.println(F("\ndeserializeJson() failed!"));
    savePrefs();
  }
  myFreq = doc["myFreq"];
  Serial.print("FQ: "); Serial.println(myFreq / 1e6);
  mySF = doc["mySF"] = mySF;
  Serial.print("SF: "); Serial.println(mySF);
  myBW = doc["myBW"];
  Serial.print("BW: "); Serial.println(myBW);
  const char *x = doc["deviceName"];
  deviceName = x;
  Serial.print("Device Name: "); Serial.println(deviceName.c_str());
}

void loop() {
}

void hexDump(uint16_t len) {
  String s = "|", t = "| |";
  Serial.println(F("  |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f |"));
  Serial.println(F("  +------------------------------------------------+ +----------------+"));
  for (uint16_t i = 0; i < len; i += 16) {
    for (uint8_t j = 0; j < 16; j++) {
      if (i + j >= len) {
        s = s + "   "; t = t + " ";
      } else {
        char c = buf[i + j];
        if (c < 16) s = s + "0";
        s = s + String(c, HEX) + " ";
        if (c < 32 || c > 127) t = t + ".";
        else t = t + (String(c));
      }
    }
    uint8_t index = i / 16;
    Serial.print(index, HEX); Serial.write('.');
    Serial.println(s + t + "|");
    s = "|"; t = "| |";
  }
  Serial.println(F("  +------------------------------------------------+ +----------------+"));
}

void savePrefs() {
  SerialUSB.println("Saving prefs:");
  StaticJsonDocument<200> doc;
  doc["myFreq"] = myFreq;
  doc["mySF"] = mySF;
  doc["myBW"] = myBW;
  doc["deviceName"] = deviceName.c_str();
  memset(buf, 0, 97);
  serializeJson(doc, (char*)buf, 97);
  hexDump(96);
  myMem.write(0, buf, 32);
  myMem.write(32, buf + 32, 32);
  myMem.write(64, buf + 64, 32);
}
