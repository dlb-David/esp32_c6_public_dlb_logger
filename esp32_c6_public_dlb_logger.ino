/**
 * MiT licence
 *
  ToDo -> log in to wifi network, get the fingerpint from the server dlb.com.pl, update firmware if available
  Update only in no encrpyt mode
 */
int firmware_version = 2;
int server_firmware_version = 0;

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>


String server_fingerprit;
WiFiMulti wifiMulti;

void setup() {

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

    if((wifiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;
        Serial.print("[HTTP] begin...\n");

        http.begin("http://dlb.com.pl/api.php?name=dlb&command=fingerprint&device="+String(WiFi.macAddress()));
        //http.begin("http://dlb.com.pl", 80, "/api.php");

        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if(httpCode > 0) {
            // HTTP header
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            if(httpCode == HTTP_CODE_OK) {
                int len = http.getSize();
                uint8_t buff[128] = { 0 };
                WiFiClient * stream = http.getStreamPtr();

                while(http.connected() && (len > 0 || len == -1)) {
                    size_t size = stream->available();
                    if(size) {
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        Serial.write(buff, c);
                        if(len > 0) {
                            len -= c;
                        }
                    }
                    delay(2);
                }
                JsonDocument doc;
                deserializeJson(doc, buff); //buff from server = {"result":[df6238cc603929ca39cce627de1d0cabd8007c83,2]}

                const char* server_result_   = doc["result"][0];
                int server_firmware_version_ = doc["result"][1];

                server_firmware_version = server_firmware_version_;

               // server_fingerprit = String((char * )buff);
                Serial.println();
                Serial.println(server_fingerprit);
                Serial.println("END");
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();

      if(server_firmware_version>firmware_version){
        //OTA REMOTE UPDATE
        WiFiClient client;
            t_httpUpdate_return ret = httpUpdate.update(client, "http://dlb.com.pl/update"+String(server_firmware_version)+".bin"); 

            switch (ret) {
              case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                break;

              case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                break;

              case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK");
                break;
            }
      }

    }
    delay(5000);
}

