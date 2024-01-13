/**
 * MiT licence
 * https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json

  remote update user@user-MacBook-Air esp32.esp32.esp32c6 % scp esp32_c6_public_dlb_logger.ino.bin user@dlb.one:/var/www/html/update2.bin
  
  ToDo -> log in to wifi network, get the fingerpint from the server dlb.com.pl, update firmware if available
  Update only in no encrpyt mode
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include "dlb_glob.h"
#include "dlb_server.h"
#include "dlb_logger.h"
#include "dlb_clock.h"

String server_fingerprit;
WiFiMulti wifiMulti;

int server_firmware_version = 2;


dlb_glob dlb_glob_obj(10);
dlb_server dlb_server_obj;
dlb_logger* dlb_logger_obj;
dlb_clock dlb_clock_obj(0, 0, 0, 0, 0, 0, 0);

void setup() {
  Serial.begin(115200);
  Serial.println();
  neopixelWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);  // Blue
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);  //only for OTA Update
  wifiMulti.addAP("dlb", "www.dlb.one");
}

void loop() {
  if ((wifiMulti.run() == WL_CONNECTED)) {
    neopixelWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);  // Green WORK

    if (!dlb_server_obj.have_fingerprint) dlb_server_obj.get_credential("http://dlb.com.pl/api.php?name=dlb&command=fingerprint&device=" + String(WiFi.macAddress()));

    dlb_clock_obj.set_time_from_server(dlb_server_obj.get_http_buff("http://dlb.com.pl/api.php?name=dlb&command=TIME"));

    delay(1000);

    //IO PORSTS
    dlb_server_obj.send_event("http://www.dlb.com.pl/api.php?user_name=lukasz&port=1");
    delay(1000);
    dlb_server_obj.send_event("http://www.dlb.com.pl/api.php?user_name=lukasz&port=2");
    delay(1000);
    dlb_server_obj.send_event("http://www.dlb.com.pl/api.php?user_name=lukasz&port=3");
    delay(1000);

    //Analog PORTS

    //Humidity

    //Pressure

    //OLED status

    //only for remote update user@user-MacBook-Air esp32.esp32.esp32c6 % scp esp32_c6_public_dlb_logger.ino.bin user@dlb.one:/var/www/html/update2.bin
    if (Serial.read() == 'x') {
      Serial.println();
      Serial.println("update ...");
      neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);  // Red UPDATE
      //Serial.flush();
      dlb_server_obj.update("http://192.168.0.197/update" + String(server_firmware_version) + ".bin");
    }
  } else {
    neopixelWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);  // Blue NO INTERNET
  }
}
