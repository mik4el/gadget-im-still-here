/*
 *  HTTP over TLS (HTTPS) example sketch
 *
 *  This example demonstrates how to use
 *  WiFiClientSecure class to access HTTPS API.
 *  We fetch and display the status of
 *  esp8266/Arduino project continuous integration
 *  build.
 *
 *  Created by Ivan Grokhotkov, 2015.
 *  This example is in public domain.
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "WiFiConfig.h"
#include <aJSON.h>
#include <EEPROM.h>

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* host = HOST;
const int httpsPort = 443;
long messageId = 0;
String jwt;
bool authorized = false;
bool posted_message = false;
HTTPClient http;


//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to address + 3.
void EEPROMWritelong(int address, long value) {
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
  EEPROM.commit();
}

//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to address + 3.
long EEPROMReadlong(long address) {
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);
  
  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) +((one << 24) & 0xFFFFFFFF);
}

void updateMessageId() {
    EEPROM.begin(4);
    messageId = EEPROMReadlong(0) + 1;
    EEPROMWritelong(0, messageId);
    EEPROM.end();
 }

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  getJWToken();
  
}

void getJWToken() {

  Serial.println("preparing jwt auth post");

  aJsonObject *root;
  root=aJson.createObject();
  aJson.addStringToObject(root, "username", HOST_USERNAME);
  aJson.addStringToObject(root, "password", HOST_PASSWORD);
  char *payload=aJson.print(root);

  Serial.println("making jwt auth post");

  http.begin(HOST_JWT_URL, HOST_FINGERPRINT);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST((uint8_t *)payload,strlen(payload));

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      //read and save json token
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
          char payload_char[payload.length()+1]; 
          payload.toCharArray(payload_char, payload.length()+1);
          root = aJson.parse(payload_char);
          aJsonObject* token = aJson.getObjectItem(root, "token");
          jwt = token->valuestring;
          authorized = true;
          Serial.println(jwt);
      }
  } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

}

void postImStillHere() {

  Serial.println("Preparing I'm still here post");
  
  updateMessageId();
  
  aJsonObject *root, *data_json;
  root=aJson.createObject();
  aJson.addItemToObject(root, "data", data_json = aJson.createObject());
  aJson.addStringToObject(data_json, "ssid", WIFI_SSID);
  aJson.addNumberToObject(data_json, "message_id", (int)messageId);
  char *payload=aJson.print(root);
  Serial.println(payload);

  Serial.println("Making I'm still here post");

  http.begin(HOST_POST_URL, HOST_FINGERPRINT);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + jwt);
  int httpCode = http.POST((uint8_t *)payload,strlen(payload));

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      posted_message = true;
  } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      authorized = false;
  }

  http.end();
}

void loop() {
  if (authorized) {
    if (posted_message) {
      ESP.deepSleep(1000);  // Dies since no interrupt pin on esp-01    
    } else {
      Serial.println("Authorized, sending data.");
      postImStillHere();
    }
  } else {
    Serial.println("Not authorized!");  
    getJWToken();
    delay(1000);
  }
}
