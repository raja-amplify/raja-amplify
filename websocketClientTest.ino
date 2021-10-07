#include <ArduinoWebsockets.h>
#include <WiFi.h>

const char* ssid = "EVRE_R&D"; //Enter SSID
const char* password = "Amplify5"; //Enter Password
const char* websockets_server = "wss://ws.semaconnect.in/D0F20E3950800000"; //server adress and port
//const char* websockets_server = "ws://evcp.evsecerebro.com:8140/websocket/amplify/9100021";
using namespace websockets;

const char ssl_ca_cert[] PROGMEM = \
    "-----BEGIN CERTIFICATE-----\n" \
    "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" \
    "MqKJ7b8bwdeAR+C260e//TbdnBa0YZhYPaG2" \
    "-----END CERTIFICATE-----\n";



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
    else
    {
      Serial.println("Something else");
    }
}

WebsocketsClient client;
void setup() {
    Serial.begin(115200);
    // Connect to wifi
    Serial.println("Checking for wifi");
    WiFi.begin(ssid, password);Serial.println(WiFi.SSID());
 
    delay(1000);
       // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("D1-connected to wifi");
    Serial.println(WiFi.SSID());
      //client.setCACert(ssl_ca_cert);
    // Setup Callbacks
    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);
    
    // Connect to server
    client.connect(websockets_server);

    // Send a message
    client.send("[2,\"531531531\",\"BootNotification\",{\"chargePointVendor\":\"Agrawal\",\"chargePointSerialNumber\":\"dummyCP002\",\"chargePointModel\":\"Pulkit\"}]");
    // Send a ping
    client.ping();
}

void loop() {
  //Serial.println("Debug");
 // client.send("[2,\"531531531\",\"BootNotification\",{\"chargePointVendor\":\"Agrawal\",\"chargePointSerialNumber\":\"dummyCP002\",\"chargePointModel\":\"Pulkit\"}]");
    // Send a ping
   // client.ping();
    client.poll();
}
