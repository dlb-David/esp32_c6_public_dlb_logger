/**
 * MiT licence
 * https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json

  remote update user@user-MacBook-Air esp32.esp32.esp32c6 % scp esp32_c6_public_dlb_logger.ino.bin user@dlb.one:/var/www/html/update2.bin

  ToDo -> log in to wifi network, get the fingerpint from the server dlb.com.pl, update firmware if available
  Update only in no encrpyt mode
  PIN2 - ANALOG 2
  PIN3 - ANALOG 3
  PIN8 - LED

  PIN22 - I2C CLK
  PIN23 - I2C DATA
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include "mbedtls/md.h"  //SHA1 -> fingerprint
#include "dlb_glob.h"
#include "dlb_server.h"
#include "dlb_logger.h"
#include "dlb_clock.h"
#include "dlb_eeprom.h"
#include "dlb_LCD.h"
#include "dlb_OLED.h"

#include <Time.h> //watchdog and timer
#include <Ticker.h>

///////////////////////////OLED////////////////////////////////////////////////////////////
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool LCD_OLED=true;
///////////////////////////OLED////////////////////////////////////////////////////////////

int server_firmware_version = 42;

String server_fingerprit;
WiFiMulti wifiMulti;
WebServer server(80);

const char *www_username = "admin";
const char *www_password = "dlb";


long int d60_energy=0;
int n_cycle=0;

dlb_glob dlb_glob_obj(10);
dlb_server dlb_server_obj(&server, server_firmware_version);
dlb_logger dlb_logger_obj;
dlb_clock dlb_clock_obj(0, 0, 0, 0, 0, 0, 0);
dlb_eeprom dlb_eeprom_obj;
dlb_LCD dlb_LCD_obj;
dlb_OLED dlb_OLED_obj;


struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};
/*
Button button1 = { 23, 0, false };
Button button2 = { 22, 0, false };
Button button3 = { 21, 0, false };
*/
Button button1 = { 10, 0, false };
Button button2 = { 20, 0, false };
Button button3 = { 11, 0, false };

void IRAM_ATTR I_OK_pressed() {
  dlb_logger_obj.P_up(1);
}

void IRAM_ATTR I_NOK_pressed() {
  dlb_logger_obj.P_up(2);
}

void IRAM_ATTR I_POP_pressed() {
  dlb_logger_obj.P_up(3);
}

Ticker secoundTick;
volatile int watchdogCount = 0;

void ISRwatchdog() {
  watchdogCount ++;
  Serial.println(watchdogCount);
  Serial.println("*");
  if (watchdogCount == 45) {
    //only print to serial when debugging
    //(debug) && Serial.println("Pies uciekl !");
    esp_restart();
  }
 //pomiar prądu żużycia energii
 //POWER_dt=POWER_dt+getToutValue();
 //if (POWER_dt>2000000000) POWER_dt=0;
}

void setup() {
  Serial.begin(115200);

  secoundTick.attach(1, ISRwatchdog);

  if(LCD_OLED) {
      if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
        LCD_OLED = false; //no more LCD
      }
      else
      {
        display.clearDisplay();       
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(26, 6);
        display.print("www.");
        display.setTextSize(2);
        display.setCursor(51, 0);
        display.print("dlb");
        display.setTextSize(1);
        display.setCursor(85, 6);
        display.println(".one");
        display.setTextSize(1);
        display.print("v"+String(server_firmware_version)+" wait");
        display.display();
      }
  }

  //set the resolution to 12 bits (0-4096)
  analogReadResolution(12);


  //-----------------------------------------------------------------------------------------------------HASH
  char *key = "secretKey";
  char *payload = "Hello HMAC SHA 256!";
  byte hmacResult[32];

  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  const size_t payloadLength = strlen(payload);
  const size_t keyLength = strlen(key);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char *)key, keyLength);
  mbedtls_md_hmac_update(&ctx, (const unsigned char *)payload, payloadLength);
  mbedtls_md_hmac_finish(&ctx, hmacResult);
  mbedtls_md_free(&ctx);

  Serial.print("Hash: ");  // -> Hash: 40f08b93b298f788109624ad3505882e0467fab1b30a76993a8327097c4f4e45

  for (int i = 0; i < sizeof(hmacResult); i++) {
    char str[3];
    sprintf(str, "%02x", (int)hmacResult[i]);
    Serial.print(str);
  }
  //-----------------------------------------------------------------------------------------------------//HASH

  pinMode(button1.PIN, INPUT_PULLUP);
  pinMode(button2.PIN, INPUT_PULLUP);
  pinMode(button3.PIN, INPUT_PULLUP);

  attachInterrupt(button1.PIN, I_OK_pressed, RISING);
  attachInterrupt(button2.PIN, I_NOK_pressed, FALLING);
  attachInterrupt(button3.PIN, I_POP_pressed, FALLING);

  Serial.println();
  neopixelWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);  // Blue

  WiFi.mode(WIFI_STA);  //only for OTA Update
  wifiMulti.addAP("dlb", "www.dlb.one");

  server.on("/", []() {
    if (!server.authenticate(www_username, www_password)) {
      return server.requestAuthentication();
    }
    server.send(200, "text/plain", "Login OK");
  });
  server.begin();
  while (!(wifiMulti.run() == WL_CONNECTED)) {
  }
  //
  Serial.print("check update ... -> http://192.168.0.197/update");
  Serial.print((server_firmware_version+1));
  Serial.println(".bin");
  neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);  // Red UPDATE
  dlb_server_obj.update("http://192.168.0.197/update" + String(server_firmware_version + 1) + ".bin");
}

void loop() {
  server.handleClient();

  delay(500);
 
  //read analogs
  int analog_1 = analogReadMilliVolts(1); // IO1 PIN9   //CH0 <temperature>
  int analog_2 = analogReadMilliVolts(2); // IO2 PIN27  //CH0 <temperature>
  int analog_3 = analogReadMilliVolts(3); // IO3 PIN26  //CH1 <pressure>  PIN3
  int analog_4 = analogReadMilliVolts(4); // IO4 PIN4   //CH2 <energy>    PIN4
  int analog_5 = analogReadMilliVolts(5); // IO5 PIN5   //CH3 <sigpower>  

  d60_energy += analog_4;
  n_cycle++;

  if(LCD_OLED) {
    display.clearDisplay();       
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(51, 0);
    display.println(n_cycle);
    display.setTextSize(1);
    display.println("P_"+String(analog_3)+"/E_"+String(analog_4));
    display.println(" RF -> "+String(WiFi.RSSI())+" dB");
    display.display();
  }

  if ((wifiMulti.run() == WL_CONNECTED)) {

    watchdogCount=0;

    neopixelWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);  // Green WORK

    if (!dlb_server_obj.have_fingerprint) {
      dlb_server_obj.get_credential("http://dlb.com.pl/api.php?name=dlb&command=fingerprint&device=" + String(WiFi.macAddress()));
      dlb_clock_obj.set_time_from_server(dlb_server_obj.get_http_buff("http://dlb.com.pl/api.php?name=dlb&command=TIME"));
      dlb_server_obj.update("http://192.168.0.197/update" + String(server_firmware_version + 1) + ".bin");
      
      Serial.print("Open http://");
      Serial.print(WiFi.localIP());
      Serial.println("/ in your browser to see it working");
    }

    //delay(500);

    //IO PORSTS
   // dlb_server_obj.send_event("http://www.dlb.com.pl/api.php?user_name=lukasz&port=1");
   // delay(1000);
   // dlb_server_obj.send_event("http://www.dlb.com.pl/api.php?user_name=lukasz&port=2");
   // delay(1000);
   // dlb_server_obj.send_event("http://www.dlb.com.pl/api.php?user_name=lukasz&port=3");
    //delay(1000);

    Serial.printf("ADC analog_1 = %d, analog_2 = %d, analog_3 = %d, analog_4 = %d, analog_5 = %d, d60_energy = %d\n\n", analog_1, analog_2, analog_3, analog_4, analog_5, d60_energy);

    if(n_cycle>120) dlb_server_obj.send_event("http://www.dlb.com.pl/api.php?user_name=lukasz&port=1&analog=A0A"+String(analog_3)+"A"+String(d60_energy)+"A"+String(WiFi.RSSI())+"A");

    //Humidity

    //Pressure

    //OLED LCD status

    //only for remote update user@user-MacBook-Air esp32.esp32.esp32c6 % scp esp32_c6_public_dlb_logger.ino.bin user@dlb.one:/var/www/html/update2.bin
    if (Serial.read() == 'x') {  //force update
      Serial.println();
      Serial.println("update ...");
      neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);  // Red UPDATE
      //Serial.flush();
      dlb_server_obj.update("http://192.168.0.197/update" + String(server_firmware_version) + ".bin");  //last old version
    }
  } else {
    neopixelWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);  // Blue NO INTERNET
  }

  if(n_cycle>120) {n_cycle=0; d60_energy=0;}
}
