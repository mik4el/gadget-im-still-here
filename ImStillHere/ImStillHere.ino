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

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* host = "m4bd.se";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
#define FINGERPRINT "81 B2 CE B2 BB 60 8E AC FA BC 8D A3 66 4F 68 1A 23 BB 54 EB"

String jwt;
bool authorized = false;
HTTPClient http;

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

  http.begin("https://m4bd.se/backend/api/v1/gadgets/", FINGERPRINT);
  
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  getJWToken();
  if (authorized) {
    Serial.println("Authorized, sending data.");
    postImStillHere();
  } else {
    Serial.println("Not authorized!");  
  }

}

void getJWToken() {

  Serial.println("preparing jwt auth post");

  aJsonObject *root;
  root=aJson.createObject();
  aJson.addStringToObject(root, "username", HOST_USERNAME);
  aJson.addStringToObject(root, "password", HOST_PASSWORD);
  char *payload=aJson.print(root);

  Serial.println("making jwt auth post");

  http.begin("https://m4bd.se/backend/api-token-auth/", FINGERPRINT);
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

  aJsonObject *root, *data_json;
  root=aJson.createObject();
  aJson.addItemToObject(root, "data", data_json = aJson.createObject());
  aJson.addStringToObject(data_json, "ssid", WIFI_SSID);
  //add message id to data_json
  char *payload=aJson.print(root);
  Serial.println(payload);

  Serial.println("Making I'm still here post");

  http.begin("https://m4bd.se/backend/api/v1/gadgets/im-still-here/data/", FINGERPRINT);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + jwt);
  int httpCode = http.POST((uint8_t *)payload,strlen(payload));

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      //read and save json token
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void loop() {
}
