#include <ArduinoJson.h>
#include <CRC32.h>
#include <Update.h>
#include "FS.h"
#include "SPIFFS.h"
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
//#include <TinyGsmClientSecure.h>
String apn = "m2misafe";                    //APN
const char apni[]  = "m2misafe";
String apn_u = "";                     //APN-Username
String apn_p = "";                     //APN-Password
const char gprsUser[] = "";
const char gprsPass[] = "";
String url = "us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate";  //URL of Server
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200
//SoftwareSerial SWserial(8, 9); // RX, TX
#define DEBUG_OUT
#define SWserial Serial2
//#define SerialAT Serial1
#define SerialAT Serial2
#define SerialMon Serial
TinyGsm        modem(SerialAT);
TinyGsmClientSecure sclient(modem);
TinyGsmClientSecure client(modem);
#define TINY_GSM_USE_GPRS true
const char server[] = "https://vesso.in";
const int port = 443;
const char resource[] = "/Blink.bin"; //here de bin file

uint32_t knownCRC32 = 0Xbd79ed5e;
uint32_t knownFileSize = 1024; // In case server does not send it
void setup()
{
  Serial.begin(115200);
  Serial.println("SIM800 AT CMD Test");
  //SWserial.begin(9600);
  TinyGsmAutoBaud(SWserial, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  delay(3000);
  while (SWserial.available()) {
    Serial.write(SWserial.read());
  }
  delay(2000);
  SetupGsm();
  //gsm_config_gprs();

  if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    SPIFFS.format();
    listDir(SPIFFS, "/", 0);

    // Set GSM module baud rate
    
    delay(3000);
}

void loop() {
  DynamicJsonDocument object(1024);
  object["version"] = "POD_003.ino.esp32";
  object["deviceId"] = "CP001";
  String sendtoserver;
  serializeJson(object, sendtoserver);
 // gsm_http_post(sendtoserver);
 httpsPost(sendtoserver);
  delay(30000);
  //ota_update();
}

void ota_update()
{
  Serial.print("Connecting to ");
    Serial.print(server);

    // if you get a connection, report back via serial:
   if (!client.connect(server, port))
    {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    //Serial.println(" OK");
    
    // Make a HTTP request:
    client.print(String("GET ") + resource + " HTTP/1.0\r\n");
    client.print(String("Host: ") + server + "\r\n");
    client.print("Connection: keep-alive\r\n\r\n");

    long timeout = millis();
    while (client.available() == 0)
    {
        if (millis() - timeout > 15000L)
        {
            Serial.println(">>> Client Timeout !");
            client.stop();
            delay(10000L);
            return;
        }
    }

    Serial.println("Reading header");
    uint32_t contentLength = knownFileSize;

    File file = SPIFFS.open("/update.bin", FILE_APPEND);


    while (client.available())
    {
        String line = client.readStringUntil('\n');
        line.trim();
        //Serial.println(line);    // Uncomment this to show response header
        line.toLowerCase();
        if (line.startsWith("content-length:"))
        {
            contentLength = line.substring(line.lastIndexOf(':') + 1).toInt();
        }
        else if (line.length() == 0)
        {
            break;
        }
    }


    timeout = millis();
    uint32_t readLength = 0;
    CRC32 crc;

    unsigned long timeElapsed = millis();
    printPercent(readLength, contentLength);
    

    while (readLength < contentLength && client.connected() && millis() - timeout < 10000L)
    {
        int i = 0;
        while (client.available())
        {
                // read file data to spiffs
            if (!file.print(char(client.read())))
            {
                Serial.println("Appending file");
            }
            //Serial.print((char)c);       // Uncomment this to show data
            //crc.update(c);
            readLength++;

            if (readLength % (contentLength / 13) == 0)
            {
                printPercent(readLength, contentLength);
            }
            timeout = millis();
        }
    }

    file.close();

    printPercent(readLength, contentLength);
    timeElapsed = millis() - timeElapsed;
    Serial.println();

    client.stop();
    Serial.println("stop client");

    modem.gprsDisconnect();
    Serial.println("gprs disconnect");
    Serial.println();

    float duration = float(timeElapsed) / 1000;
  /*
    Serial.print("Tamaño de Archivo: ");
    Serial.println(contentLength);
    Serial.print("Leido:  ");
    Serial.println(readLength);
    Serial.print("Calculado. CRC32:    0x");
    Serial.println(crc.finalize(), HEX);
    Serial.print("Conocido CRC32:    0x");
    Serial.println(knownCRC32, HEX);
    Serial.print("Bajado en:       ");
    Serial.print(duration);
    Serial.println("s");

    Serial.println("Se genera una espera de 3 segundos");
    for (int i = 0; i < 3; i++)
    {
        Serial.print(String(i) + "...");
        delay(1000);
    }
  */
    //readFile(SPIFFS, "/update.bin");

    updateFromFS();

    // Do nothing forevermore
    while (true)
    {
        delay(1000);
    }
}

void gsm_http_post( String sendtoserver) {

  
  Serial.println(" --- Start GPRS & HTTP --- ");
  //gsm_send_serial("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  //gsm_send_serial("AT+SAPBR=3,1,\"APN\",\"m2misafe\"");
  //gsm_send_serial("AT+SAPBR=1,1");
  //gsm_send_serial("AT+SAPBR=2,1");
  gsm_send_serial("AT+HTTPINIT");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + url);
  gsm_send_serial("AT+HTTPPARA=CONTENT,application/json");
  //gsm_send_serial("AT+HTTPPARA=CONNECTION,keep-alive");
  //gsm_send_serial("AT+HTTPDATA=" + String(sendtoserver.length()) + ",100000");
  gsm_send_serial("AT+HTTPDATA=55,100000");
  gsm_send_serial(sendtoserver);
  gsm_send_serial("AT+HTTPSSL=1");
  gsm_send_serial("AT+HTTPACTION=1"); // 1 signifies post 0 signifies get
  gsm_send_serial("AT+HTTPREAD");
  delay(30000);
  gsm_send_serial("AT+HTTPTERM");
  gsm_send_serial("AT+SAPBR=0,1");
}

void gsm_config_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");
  gsm_send_serial("AT+SAPBR=3,1,Contype,GPRS");
  gsm_send_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    gsm_send_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    gsm_send_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}

void gsm_send_serial(String command) {
  Serial.println("Send ->: " + command);
  SWserial.println(command);
  long wtimer = millis();
  while (wtimer + 10000 > millis()) {
    while (SWserial.available()) {
      Serial.write(SWserial.read());
    }
  }
  Serial.println();
}

void httpsPost(String httpPostData)
{

   if (!sclient.connect("https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate", 443)) 
    {
      Serial.println(" fail");
    }
      Serial.println(F("posting"));
      sclient.print(String("POST /otaUpdate?CONTENT=application/json HTTP/1.1\r\n"));
      sclient.print(String("Host: us-central1-evre-iot-308216.cloudfunctions.net\r\n"));
      sclient.println("Connection: keep-alive\r\n");
      sclient.println("Content-Type: application/json\r\n");
      sclient.println("Content-Length: 55\r\n");
     // sclient.println(httpPostData.length()+"\r\n");
      sclient.println();
      sclient.println(httpPostData+"\r\n");
      unsigned long timeout = millis();
      String response = "";
      long int time = millis();
      while ((time + 10000) > millis())
      {
        //Serial.println(F("sclient unavailabale"));
          while (sclient.available())
          {
              char c = sclient.read();
              response += c;
              Serial.print("Got resp:");
              Serial.println(response);
          }
      }
      Serial.print(F("Out of while"));
      Serial.println(response);
      
}

void SetupGsm(){

    Serial.println("[CustomGsm] Starting GSM Setup");
  //  TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
    delay(200);
    Serial.println("[CustomGsm] Initializing modem...");
    modem.restart();

    String modemInfo = modem.getModemInfo();
    Serial.print("[CustomGsm] Modem Info: ");
    Serial.println(modemInfo);

    SerialMon.println("[CustomGsm] Waiting for network...");
    if (!modem.waitForNetwork()) {
        SerialMon.println("[CustomGsm] fail");
        delay(200);
        return;
  }
  SerialMon.println("[CustomGsm] success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("[CustomGsm] Network connected");
  }


    SerialMon.print(F("[CustomGsm] Connecting to "));
    SerialMon.print(apni);
    if (!modem.gprsConnect(apni, gprsUser, gprsPass)) {
       SerialMon.println("[CustomGsm] fail");
      delay(200);
      return;
    }
    SerialMon.println("[CustomGsm] success");

  if (modem.isGprsConnected()) {
   SerialMon.println("[CustomGsm]GPRS connected");
  }
 
    int csq = modem.getSignalQuality();
    Serial.println("Signal quality: "+String(csq));
    delay(1);




}


void appendFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("APOK");
    }
    else
    {
        Serial.println("APX");
    }
}

void readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available())
    {
        Serial.write(file.read());
        delayMicroseconds(100);
    }
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("File written");
    }
    else
    {
        Serial.println("Write failed");
    }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void deleteFile(fs::FS &fs, const char *path)
{
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path))
    {
        Serial.println("File deleted");
    }
    else
    {
        Serial.println("Delete failed");
    }
}

void updateFromFS()
{
    File updateBin = SPIFFS.open("/update.bin");
    if (updateBin)
    {
        if (updateBin.isDirectory())
        {
            Serial.println("Directory error");
            updateBin.close();
            return;
        }

        size_t updateSize = updateBin.size();

        if (updateSize > 0)
        {
            Serial.println("Starting update");
            performUpdate(updateBin, updateSize);
        }
        else
        {
            Serial.println("Error, archivo vacío");
        }

        updateBin.close();

        // whe finished remove the binary from sd card to indicate end of the process
        //fs.remove("/update.bin");
    }
    else
    {
        Serial.println("no such binary");
    }
}

void performUpdate(Stream &updateSource, size_t updateSize)
{
    if (Update.begin(updateSize))
    {
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize)
        {
            Serial.println("Writes : " + String(written) + " successfully");
        }
        else
        {
            Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
        }
        if (Update.end())
        {
            Serial.println("OTA finished!");
            if (Update.isFinished())
            {
                Serial.println("Restart ESP device!");
                ESP.restart();
            }
            else
            {
                Serial.println("OTA not fiished");
            }
        }
        else
        {
            Serial.println("Error occured #: " + String(Update.getError()));
        }
    }
    else
    {
        Serial.println("Cannot beggin update");
    }
}





void printPercent(uint32_t readLength, uint32_t contentLength)
{
    // If we know the total length
    if (contentLength != -1)
    {
        Serial.print("\r ");
        Serial.print((100.0 * readLength) / contentLength);
        Serial.print('%');
    }
    else
    {
        Serial.println(readLength);
    }
}
