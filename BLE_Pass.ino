/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "6e9d6fb7-d8bb-44f4-b006-5f0cc516a8b0"
#define SSID_CHARACTERISTIC_UUID "2fb28235-34e4-4470-b880-db01eeb6eac8" // ssid and password characteristic

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;


class MySecurity : public BLESecurityCallbacks {
  
  bool onConfirmPIN(uint32_t pin){
    Serial.println(F("confirming pin"));
    return false;
  }
  
  uint32_t onPassKeyRequest(){
        Serial.println(F("PassKeyRequest"));
        ESP_LOGI(LOG_TAG, "PassKeyRequest");
    return 123456;
  }

  void onPassKeyNotify(uint32_t pass_key){
        Serial.print(F("On passkey Notify number:"));
        Serial.println(pass_key);
        ESP_LOGI(LOG_TAG, "On passkey Notify number:%d", pass_key);
  }

  bool onSecurityRequest(){
    Serial.println(F("On Security Request"));
      ESP_LOGI(LOG_TAG, "On Security Request");
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl){
    ESP_LOGI(LOG_TAG, "Starting BLE work!");
    if(cmpl.success){
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
void setup() {
  Serial.begin(115200);
  Serial.println("Starting Commissioning.");

  BLEDevice::init("Amplify-Commissioning");
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  /*
   * Required in authentication process to provide displaying and/or input passkey or yes/no butttons confirmation
   */
  BLEDevice::setSecurityCallbacks(new MySecurity());
  /*BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );*/

  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         SSID_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_INDICATE|
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );


  pCharacteristic->setValue("Write SSID/Pwd in 12 bytes");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  //Use the below code for BLE password protected approach.
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
  pSecurity->setCapability(ESP_IO_CAP_OUT);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  // This works
  uint32_t passkey = 1427;
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  
  Serial.println("SSID and Password characteristic defined. Write it in the client to update.");
}

void loop() {
  // put your main code here, to run repeatedly:
  std::string value = pCharacteristic->getValue();
  Serial.print(F("SSID and Pwd: "));
  Serial.println(F(value.c_str()));
  delay(2000);
}
