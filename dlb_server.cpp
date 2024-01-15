#include "dlb_server.h"

dlb_server::dlb_server(WebServer* server, int logger_firmware_version) {
  this->server_ = server;
  logger_firmware_version_ = logger_firmware_version;
}


bool dlb_server::send_event(String addres) {
  String stringOne = get_http_buff(addres);
  //{"result":GET OK}
  if (stringOne.indexOf("GET OK") > 3) return true;
  else return false;
}

bool dlb_server::get_credential(String addres) {

  String stringOne = get_http_buff(addres);
  //Serial.println(stringOne);

  int firstClosingBracket = stringOne.indexOf('[');
  //Serial.printf("firstClosingBracket lock -> %d",firstClosingBracket);
  firstClosingBracket += 1;
  int lock = firstClosingBracket;
  String pom_fingerprint;
  int len = stringOne.length();
  while (lock < (len - 2)) {
    pom_fingerprint = pom_fingerprint + stringOne[lock];
    if (stringOne[lock + 1] == ',') lock = 10000;
    lock++;
  }
  pom_fingerprint = pom_fingerprint + '\0';

  if (pom_fingerprint.length() > 10) {
    this->have_fingerprint = true;
    pom_fingerprint.toCharArray(this->fingerprint, pom_fingerprint.length());
  }

  firstClosingBracket = stringOne.indexOf(',');
  lock = firstClosingBracket + 1;
  String pom_version;
  while (lock < (len - 2)) {
    pom_version = pom_version + stringOne[lock];
    if (stringOne[lock + 1] == ']') lock = 10000;
    lock++;
  }
  this->update_version = pom_version.toInt();

  if (pom_version.length() > 0) have_update_version = true;
  Serial.println();
  Serial.println("----------------------------------------------------------------");
  Serial.print("| fingerprint from server -> ");
  Serial.println(this->fingerprint);
  Serial.print("| version from server -> ");
  Serial.println(this->update_version);
  Serial.print("| version from logger -> ");
  Serial.println(this->logger_firmware_version_);
  Serial.println("----------------------------------------------------------------");
  Serial.println();
  Serial.println("END");

  if (this->update_version > this->logger_firmware_version_) {
    Serial.println();
    Serial.println("new auto remote update ...");
    neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);  // Red UPDATE
    //Serial.flush();
    update("http:/dlb.sytes.net/update" + String(this->update_version) + ".bin");
  }

  return (have_fingerprint and have_update_version);
}

bool dlb_server::update(String addres) {
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

void dlb_server::set_macAddress() {
}



String dlb_server::get_http_buff(String addres) {
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");

  http.begin(addres);
  //http.begin("http://dlb.com.pl", 80, "/api.php");

  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();
  if (httpCode > 0) {
    // HTTP header
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      int len = http.getSize();
      uint8_t buff[128] = { 0 };
      WiFiClient* stream = http.getStreamPtr();

      while (http.connected() && (len > 0 || len == -1)) {
        size_t size = stream->available();
        if (size) {
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          Serial.write(buff, c);
          if (len > 0) {
            len -= c;
          }
        }
        delay(2);  //tooo slow ...
      }

      return (char*)buff;

    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      return "[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str();
    }

    http.end();
  }
}