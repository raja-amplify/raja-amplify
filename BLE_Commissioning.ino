#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <nvs_flash.h>
#include <Preferences.h>
Preferences preferences;
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic_AC2M = NULL;
BLECharacteristic* pCharacteristic_M2AC = NULL;
std::string Value ;

//#define SERVICE_UUID        "31b575ff-cc00-49b9-9aec-b8e5e2023b27"
#define SERVICE_UUID          "06122111-0820-20A1-20B2-40F520AB4C86"

//#define AC2M_UUID "d6fa0c77-d02c-4548-9314-1faa14cb4e36"
//#define M2AC_UUID "ce102f5b-d0fe-4b6b-bb26-cb4a03e0be49"

#define AC2M_UUID "06122111-0820-20A1-20B2-40F520AB4C87"
#define M2AC_UUID "06122111-0820-20A1-20B2-40F520AB4C88"

bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


class MySecurity : public BLESecurityCallbacks {

    bool onConfirmPIN(uint32_t pin) {
      Serial.println(F("confirming pin"));
      return false;
    }

    uint32_t onPassKeyRequest() {
      Serial.println(F("PassKeyRequest"));
      ESP_LOGI(LOG_TAG, "PassKeyRequest");
      return 123456;
    }

    void onPassKeyNotify(uint32_t pass_key) {
      Serial.print(F("On passkey Notify number:"));
      Serial.println(pass_key);
      ESP_LOGI(LOG_TAG, "On passkey Notify number:%d", pass_key);
    }

    bool onSecurityRequest() {
      Serial.println(F("On Security Request"));
      ESP_LOGI(LOG_TAG, "On Security Request");
      return true;
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
      ESP_LOGI(LOG_TAG, "Starting BLE work!");
      if (cmpl.success) {
        Serial.println(F("Starting BLE work!"));
        /*uint16_t length;
          esp_ble_gap_get_whitelist_size(&length);
          ESP_LOGD(LOG_TAG, "size: %d", length);
          Serial.print(F("size"));
          Serial.println(F(length));*/

      }
      else
      {
        Serial.print(F("Failed auth"));
        Serial.println(cmpl.fail_reason);
      }
    }
};

using std::cout; using std::endl;
using std::string; using std::hex;
using std::stringstream;

void setup() {
  // put your setup code here, to run once:

  BLEDevice::init("LAC_EVRE");

  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  /*
     Required in authentication process to provide displaying and/or input passkey or yes/no butttons confirmation
  */
  BLEDevice::setSecurityCallbacks(new MySecurity());

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic : AC2M
  pCharacteristic_AC2M = pService->createCharacteristic(
                           AC2M_UUID,
                           BLECharacteristic::PROPERTY_READ   //|
                           //BLECharacteristic::PROPERTY_WRITE  //|
                           //BLECharacteristic::PROPERTY_NOTIFY |
                           //BLECharacteristic::PROPERTY_INDICATE
                         );



  // Create a BLE Characteristic : M2AC
  pCharacteristic_M2AC = pService->createCharacteristic(
                           M2AC_UUID,
                           //BLECharacteristic::PROPERTY_READ   //|
                           BLECharacteristic::PROPERTY_WRITE  //|
                           //BLECharacteristic::PROPERTY_NOTIFY |
                           //BLECharacteristic::PROPERTY_INDICATE
                         );



  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  //pCharacteristic_AC2M->addDescriptor(new BLE2902());

  //pCharacteristic_M2AC->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  //Use the below code for BLE password protected approach.
  /*  BLESecurity *pSecurity = new BLESecurity();
     pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
     pSecurity->setCapability(ESP_IO_CAP_OUT);
     pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
     // This works
     //uint32_t passkey = 1427;
     uint32_t passkey = 44582;
     esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));*/
  Serial.println(F("LEV AC Charge Point waiting for a client connection to notify"));

}

void BLE_connectionCheck()
{
  if (deviceConnected)
  {
    if (!oldDeviceConnected)
    {
      //requestLed(BLUE, START, 1);
      delay(100);
      //requestLed(GREEN, START, 1);
    }
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    //requestLed(BLUE, START, 1);
    delay(50); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println(F("start advertising"));
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

void state_readM2AC()
{
  // Read diagnostics and check the current state of the machine
  Value = pCharacteristic_M2AC->getValue();
  delay(10);
}
