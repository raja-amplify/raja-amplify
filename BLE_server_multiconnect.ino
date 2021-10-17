/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>


BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic1 = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
BLECharacteristic* pCharacteristic3 = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
std::string Value ;
int len = 0;
int v =0;
void assignValueChargingState();

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "31b575ff-cc00-49b9-9aec-b8e5e2023b27"
#define AVAILABILITY_CHARACTERISTIC_UUID "9b7889bc-e38a-40fc-be35-20a1d8104da9"
#define TID_CHARACTERISTIC_UUID "addbc682-284c-4ff1-935d-598da22a5e85"
#define METERVALUES_CHARACTERISTIC_UUID "955d9948-60b3-4663-aa17-5c9ef0873baa"



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

typedef enum {
  handshake,
  initialization,
  authorization,
  charging,
  endofcharge
} CP_State;

CP_State cp_state = handshake;
using std::cout; using std::endl;
using std::string; using std::hex;
using std::stringstream;
//#define 0b1000000 invalid_opid

struct chargingStateCM
{
  char sessionid[10] = "123456789";
  uint8_t chargeStartStop;
  uint16_t metervalue;
  uint16_t metervoltage;
  uint16_t metercurrent;
  uint16_t error_code;
  uint8_t chargingState;

};

chargingStateCM csCM;
char responsechargingState[20];


//#include "src/customBLE.h"
void setup() {
  Serial.begin(115200);


  assignValueChargingState();
  // Create the BLE Device
  BLEDevice::init("LEV AC Charge Point");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic : to check availability
  pCharacteristic1 = pService->createCharacteristic(
                       AVAILABILITY_CHARACTERISTIC_UUID,
                       BLECharacteristic::PROPERTY_READ   |
                       //BLECharacteristic::PROPERTY_WRITE  |
                       BLECharacteristic::PROPERTY_NOTIFY |
                       BLECharacteristic::PROPERTY_INDICATE
                     );

  // Create a BLE Characteristic : to write transaction id
  pCharacteristic2 = pService->createCharacteristic(
                       AVAILABILITY_CHARACTERISTIC_UUID,
                       BLECharacteristic::PROPERTY_READ   |
                       BLECharacteristic::PROPERTY_WRITE  |
                       BLECharacteristic::PROPERTY_NOTIFY |
                       BLECharacteristic::PROPERTY_INDICATE
                     );
  // Create a BLE Characteristic : to check meter values
  pCharacteristic3 = pService->createCharacteristic(
                       AVAILABILITY_CHARACTERISTIC_UUID,
                       BLECharacteristic::PROPERTY_READ   |
                       //BLECharacteristic::PROPERTY_WRITE  |
                       BLECharacteristic::PROPERTY_NOTIFY |
                       BLECharacteristic::PROPERTY_INDICATE
                     );


  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic1->addDescriptor(new BLE2902());
  pCharacteristic2->addDescriptor(new BLE2902());
  pCharacteristic3->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println(F("LEV AC Charge Point waiting for a client connection to notify"));

  pCharacteristic2->setValue((uint8_t*)&csCM, 20);
  pCharacteristic2->notify();

  //int v = x2i("003C");

  //Serial.println(v);

}

void loop() {
  /* // notify changed value
    if (deviceConnected) {
       pCharacteristic->setValue((uint8_t*)&value, 4);
       pCharacteristic->notify();
       value++;
       Value = pCharacteristic->getValue();
    Serial.println(F(Value.c_str()));
      // Serial.println(value);
       //Serial.println(F("Notify"));
       delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
       delay(500); // give the bluetooth stack the chance to get things ready
       pServer->startAdvertising(); // restart advertising
       Serial.println("start advertising");
       oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
       // do stuff here on connecting
       oldDeviceConnected = deviceConnected;
    }*/

  if (deviceConnected)
  {
    cp_state = endofcharge;
    //pCharacteristic2->setValue((uint8_t*)&csCM, 20); //perfect!
    //pCharacteristic1->setValue((uint8_t*)&cp_state, 4); // since we know that the size of this struct is 20 bytes
    //pCharacteristic2->notify();
    Value = pCharacteristic2->getValue();
    //pCharacteristic2->notify();
    //value++;
   // Serial.print(F("value of the characteristic is:"));
    //Serial.println(Value);

    for (const auto &item : Value) {
        cout << hex << int(item);
    }
    cout << endl;

    
    //len = Value.length();
/*
      if (Value.length() > 0) {
       Serial.println("*********");
       Serial.print("New value: ");
       for (int i = 0; i < Value.length(); i++) {
         Serial.print(Value[i]);
       }
       Serial.println();
       Serial.println("*********");
       

  }*/
  
 delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
}
// disconnecting
if (!deviceConnected && oldDeviceConnected) {
  delay(500); // give the bluetooth stack the chance to get things ready
  pServer->startAdvertising(); // restart advertising
  Serial.println(F("start advertising"));
  oldDeviceConnected = deviceConnected;
  cp_state = handshake;
  pCharacteristic1->setValue((uint8_t*)&cp_state, 4);
  pCharacteristic1->notify();
  delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
}
// connecting
if (deviceConnected && !oldDeviceConnected) {
  // do stuff here on connecting
  oldDeviceConnected = deviceConnected;
}


}

void assignValueChargingState()
{
  strcpy(csCM.sessionid, "sambar123");
  csCM.chargeStartStop = 0;
  csCM.metervalue = 123;
  csCM.metervoltage = 456;
  csCM.metercurrent = 789;
  csCM.error_code = 1;
  csCM.chargingState = 5;
}

int x2i(char *s) 
{
  int x = 0;
  for(;;) {
    char c = *s;
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0'; 
    }
    else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10; 
    }
    else break;
    s++;
  }
  return x;
}
