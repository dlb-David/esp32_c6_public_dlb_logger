/**
 * MiT licence
 * https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json

  ToDo -> log in to wifi network, get the fingerpint from the server dlb.com.pl, update firmware if available
  Update only in no encrpyt mode
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include "dlb_glob.h"
#include "dlb_server.h"

String server_fingerprit;
WiFiMulti wifiMulti;

int server_firmware_version=2;


dlb_glob dlb_glob_obj(10);
dlb_server dlb_server_obj;


void setup() {
    dlb_glob_obj.is_numer_or_char('x');
    Serial.begin(115200);
    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFi.mode(WIFI_STA); //for OTA Update
    wifiMulti.addAP("dlb", "www.dlb.one");
}

void loop() {

  //if (global_dlb.is_numer_or_char('x')==true) Serial.println("jest znakiem :-) ");
  if (dlb_glob_obj.is_numer_or_char('x')==true) Serial.println("jest znakiem :-) ");
  //dlb_server.set_macAddress;

    if((wifiMulti.run() == WL_CONNECTED)) {

       dlb_server_obj.get_credential("http://dlb.com.pl/api.php?name=dlb&command=fingerprint&device="+String(WiFi.macAddress()));

       delay(5000);

      if (Serial.read() == 'x') {
         Serial.flush();
         dlb_server_obj.update("http://192.168.0.197/update"+String(server_firmware_version)+".bin");
      }

    }
}

