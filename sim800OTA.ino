#define TINY_GSM_MODEM_SIM800                    // My modem is SIM7000
#define TINY_GSM_RX_BUFFER   1024                 // Set RX buffer to 1Kb or full HTTP response wont be received

#include <TinyGsmClient.h>                        // https://github.com/vshymanskyy/TinyGSM

#include <Arduino.h>                              // used for GSMFOTA update
#include "ESPFOTAupdater.h"                        // Updates firmware (this library is modified to work with GSM)

#define SerialAT  Serial1                         // Set serial for AT commands (to the module)
TinyGsm modem(SerialAT);                          // sets GSM modem to serial1
TinyGsmClient GSMclient(modem);                   // client for GSM



String fwImageURL     = "https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate/POD_003.ino.esp32";   //example of your .bin location MUST BE HTTP SITE!!! NOT HTTPS
const int port        = 443;                               // HTTP = 80
const char server[]   = "https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate";                     //  head server address (MUST BE HTTP!)

long contentLength;                                       // How many bytes of data the .bin is
bool isValidContentType = false;                          // checks if the .bin is valid


/*---------------------------SETUP-----------------------------*/

void setup() {
  Serial.begin(9600);                                         // Start serial monitor at a baud rate of 9600
  Serial.println("\n------STARTING-UP------");
  GSMOTA();
}

void connectClient() {    // Connect to your client server, for example
  while (!!!GSMclient.connect(server, port)) {
    Serial.print("*");
  }
}

/*-----------------------------LOOP-------------------------------*/

void loop() {

}

/*-------------------------GET-UPDATE-----------------------------*/

// used to extract header value from headers for ota update
String getHeaderValue(String header, String headerName) {
  return header.substring(strlen(headerName.c_str()));
}


void GSMOTA() {
  Serial.println("\n♦ ♦ Checking OTA ♦ ♦\n");
  //connectClient();                                // Connect to your client
  if (GSMclient.connect(server, port)) {
    Serial.println("Fetching bin file at: " + String(fwImageURL));    // tells where its going to get the .bin and the name its looking for

    // Get the contents of the bin file
    GSMclient.print(String("GET ") + fwImageURL + " HTTP/1.1\r\n" +
                    "Host: " + String(server) + "\r\n" +
                    "Cache-Control: no-cache\r\n" +
                    "Connection: close\r\n\r\n");


    unsigned long timeout = millis();
    while (GSMclient.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Client Timeout !");
        GSMclient.stop();
        return;
      }
    }
    // Once the response is available start reading reply
    while (GSMclient.available()) {

      // read line till /n
      String line = GSMclient.readStringUntil('\n');
      // remove space, to check if the line is end of headers
      line.trim();

      // if the the line is empty, this is end of headers
      // break the while and feed the remaining client to the Update.writeStream();
      if (!line.length()) {
        // headers ended
        break; // and get the OTA update started
      }

      // Check if the HTTP Response is 200 if not break and Exit Update
      if (line.startsWith("HTTP/1.1")) {
        if (line.indexOf("200") < 0) {
          Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
          break;
        }
      }

      // extracting headers starting with content length
      if (line.startsWith("Content-Length: ")) {
        contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
        Serial.println("Got " + String(contentLength) + " bytes from server");
      }

      // Next, the content type
      if (line.startsWith("Content-Type: ")) {
        String contentType = getHeaderValue(line, "Content-Type: ");
        Serial.println("Got " + contentType + " payload.");
        if (contentType == "application/octet-stream") {
          isValidContentType = true;
        }
      }
    }
  } else {
    Serial.println("Connection to " + String(server) + " failed. Please check your setup");
  }
  // Check what is the contentLength and if content type is `application/octet-stream`
  Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

  // check contentLength and content type
  if (contentLength && isValidContentType) {
    // Check if there is enough to OTA Update
    bool canBegin = Update.begin(contentLength);

    // If yes, begin
    if (canBegin) {
      Serial.println("Begin GSMFOTA update. This may take 2 - 25 minutes to complete depending on the connection and file size. Patience!!!");
      Serial.println("Firmware updating...");
      size_t written = Update.writeStream(GSMclient);

      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength));
      }

      if (Update.end()) {
        Serial.println("OTA done!");
        if (Update.isFinished()) {
          Serial.println("Update successfully completed. Rebooting to activate.");
          ESP.restart();
        } else {
          Serial.println("Update not finished... Something went wrong!");
        }
      } else {
        Serial.println("Error Occurred! \nError: " + String(Update.getError()) + " " + String(Update.errorString()) );
        Serial.println("Will try to update again at a different time.\n");
      }
    } else {
      // not enough space to begin OTA, check .bin size
      Serial.println("Not enough space to begin OTA\n");
      GSMclient.flush();
    }
  } else {
    Serial.println("There was no content in the response\n");
    GSMclient.flush();
  }
}
