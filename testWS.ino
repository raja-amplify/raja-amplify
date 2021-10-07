/*
  Secured Esp32 Websockets Client

  This sketch:
        1. Connects to a WiFi network
        2. Connects to a Websockets server (using WSS)
        3. Sends the websockets server a message ("Hello Server")
        4. Sends the websocket server a "ping"
        5. Prints all incoming messages while the connection is open

    NOTE:
    The sketch dosen't check or indicate about errors while connecting to 
    WiFi or to the websockets server. For full example you might want 
    to try the example named "Esp32-Client" (And use the ssl methods).

  Hardware:
        For this sketch you only need an Esp32 board.

  Created 15/02/2019
  By Gil Maimon
  https://github.com/gilmaimon/ArduinoWebsockets

*/

#include <ArduinoWebsockets.h>
#include <WiFi.h>

const char* ssid = "EVRE_R&D"; //Enter SSID
const char* password = "Amplify5"; //Enter Password

const char* websockets_connection_string = "wss://ws.semaconnect.in/D0F20E3950800000"; //Enter server adress
//const char* websockets_server = "wss://ws.semaconnect.in/D0F20E3950800000"; //server adress and port
//const char* websockets_server = "ws://evcp.evsecerebro.com:8140/websocket/amplify/9100021";
/*
// This certificate was updated 15.04.2021, issues on Mar 15th 2021, expired on June 13th 2021
const char echo_org_ssl_ca_cert[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\n" \
"MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\n" \
"AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\n" \
"jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\n" \
"Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\n" \
"U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\n" \
"gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\n" \
"/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\n" \
"oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\n" \
"BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\n" \
"ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\n" \
"p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\n" \
"AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\n" \
"Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\n" \
"LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\n" \
"r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\n" \
"AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\n" \
"ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\n" \
"S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\n" \
"qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\n" \
"O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\n" \
"UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\n" \
"-----END CERTIFICATE-----\n";
*/
/*
const char echo_org_ssl_ca_cert[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"MqKJ7b8bwdeAR+C260e//TbdnBa0YZhYPaG2\n" \
"-----END CERTIFICATE-----\n";
*/
const char echo_org_ssl_ca_cert[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

WebsocketsClient client;
void setup() {
    Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // run callback when messages are received
    client.onMessage(onMessageCallback);
    
    // run callback when events are occuring
    client.onEvent(onEventsCallback);

    // Before connecting, set the ssl fingerprint of the server
    client.setCACert(echo_org_ssl_ca_cert);

    // Connect to server
  bool result =  client.connect(websockets_connection_string);

    // Send a message
    // Send a message
    client.send("[2,\"531531531\",\"BootNotification\",{\"chargePointVendor\":\"Agrawal\",\"chargePointSerialNumber\":\"dummyCP002\",\"chargePointModel\":\"Pulkit\"}]");
 
 //   client.send("Hello Server");
Serial.print("Connection result:");
Serial.println(result);
    // Send a ping
    client.ping();
}

void loop() {
    client.poll();
}
