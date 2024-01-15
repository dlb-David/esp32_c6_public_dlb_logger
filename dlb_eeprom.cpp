#include "dlb_eeprom.h"

dlb_eeprom::dlb_eeprom(){
  read();
}

void dlb_eeprom::write(){
  EEPROMClass  SSID("eeprom0");
  EEPROMClass  PASSWORD("eeprom1");
  EEPROMClass  KEY("eeprom2");
   if (!SSID.begin(0x500)) {
    Serial.println("Failed to initialise SSID ...");
    //ESP.restart();
  }
  if (!PASSWORD.begin(0x200)) {
    Serial.println("Failed to initialise PASSWORD ...");
    //ESP.restart();
  }
  if (!KEY.begin(0x100)) {
    Serial.println("Failed to initialise KEY ...");
    //ESP.restart();
  }
  String pom;
  pom = (char *)SSID_;
  SSID.writeString(0, pom);
  pom = (char *)PASSWORD_;
  PASSWORD.writeString(0, pom);
  pom = (char *)KEY_;
  KEY.writeString(0, pom);
  //or you can put the double or int variable ->  KEY.put()
}

void dlb_eeprom::read(){
  EEPROMClass  SSID("eeprom0");
  EEPROMClass  PASSWORD("eeprom1");
  EEPROMClass  KEY("eeprom2");
   if (!SSID.begin(0x500)) {
    Serial.println("Failed to initialise SSID ...");
    //ESP.restart();
  }
  if (!PASSWORD.begin(0x200)) {
    Serial.println("Failed to initialise PASSWORD ...");
    //ESP.restart();
  }
  if (!KEY.begin(0x100)) {
    Serial.println("Failed to initialise KEY ...");
    //ESP.restart();
  }
  SSID.get(0, SSID_);
  PASSWORD.get(0, PASSWORD_);
  KEY.get(0, KEY_);
}

bool dlb_eeprom::znak(char myChar)
{
    if (isAscii(myChar)) { // tests if myChar isa letter or a number
    //Serial.println(" The char is OK");
    return true;
    }
    else {
    //Serial.println(" The char is NOK");
    return false;
    }
}