#include "dlb_server.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>

dlb_server::dlb_server() {
}

bool dlb_server::get_credential(String addres) {
        HTTPClient http;
        Serial.print("[HTTP] begin...\n");

        http.begin(addres);
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

                //JsonDocument doc;
                //deserializeJson(doc, buff); //buff from server = {"result":[df6238cc603929ca39cce627de1d0cabd8007c83,2]}
                Serial.println();
                Serial.println("rzutowanie char na stirng -> ");
                String stringOne = (char * )buff;
                Serial.println(stringOne);

                int firstClosingBracket = stringOne.indexOf('[');
                Serial.printf("firstClosingBracket lock -> %d",firstClosingBracket);
                firstClosingBracket += 1;
                int lock=firstClosingBracket;
                String pom_fingerprint;
                len=stringOne.length();
                while(lock<(len-2)){
                    pom_fingerprint = pom_fingerprint + stringOne[lock];
                    if (stringOne[lock+1]==',') lock=10000;
                    lock++;
                }

                firstClosingBracket = stringOne.indexOf(',');
                lock=firstClosingBracket+1;
                String pom_version;
                while(lock<(len-2)){
                    pom_version = pom_version + stringOne[lock];
                    if (stringOne[lock+1]==']') lock=10000;
                    lock++;
                }
                Serial.println();
                Serial.println("--------------------------------");
                Serial.print("| fingerprint from server -> ");
                Serial.println(pom_fingerprint);
                Serial.print("| version from server -> ");
                Serial.println(pom_version);
                Serial.println("--------------------------------");
                Serial.println();
                Serial.println("manual END");

                Serial.println("END");
            }

        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();

  return true;
}

bool dlb_server::update(String addres){
        //OTA REMOTE UPDATE
        WiFiClient client;
            t_httpUpdate_return ret = httpUpdate.update(client, addres);

            switch (ret) {
              case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                return false;
                break;

              case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                return false;
                break;

              case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK");
                return true;
                break;
            }
}

void dlb_server::set_macAddress(){

}