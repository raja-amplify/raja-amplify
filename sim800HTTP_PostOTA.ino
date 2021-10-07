//#include <SoftwareSerial.h>
//SoftwareSerial myserial(10, 11); // RX: 10, TX:11
//https://github.com/vshymanskyy/TinyGSM/issues/414 -> Implement this for OTA
#define myserial Serial2
#include <ArduinoJson.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <Update.h>
#include <CRC32.h>

#include "FS.h"
#include "SPIFFS.h"
//StaticJsonBuffer<200> jsonBuffer; 
char deviceID[12] = "MYTEST56";
// Increase RX buffer to capture the entire response
// Chips without internal buffering (A6/A7, ESP8266, M590)
// need enough space in the buffer for the entire response
// else data will be lost (and the http library will fail).
#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 650
#endif

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
#define SerialAT Serial1
// #define LOGGING  // <- Logging is for the HTTP library

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

// Define how you're planning to connect to the internet.
// This is only needed for this example, not in other code.
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

// flag to force SSL client authentication, if needed
// #define TINY_GSM_SSL_CLIENT_AUTHENTICATION

// Your GPRS credentials, if any
const char apn[]      = "YourAPN";
const char gprsUser[] = "";
const char gprsPass[] = "";
void test();
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif
TinyGsmClient client(modem);
uint32_t knownFileSize = 1024; // In case server does not send it
void setup() {
  // put your setup code here, to run once:
 // myserial.begin(9600);        // the GPRS baud rate
 TinyGsmAutoBaud(myserial, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  Serial.begin(57600);
  Serial.println("Initializing..........");
//  DynamicJsonBuffer jsonBuffer;
 test();
 

}


void test() {
  // put your main code here, to run repeatedly:

  /********************GSM Communication Starts********************/
 
  if (myserial.available())
  Serial.write(myserial.read());
 
  myserial.println("AT");
  delay(1000);

  myserial.println("AT+IPR=57600"); // check the baud rate
  delay(1000);
  ShowSerialData();
   
 
  myserial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  delay(1000);
  ShowSerialData();
 
  myserial.println("AT+SAPBR=3,1,\"APN\",\"m2misafe\"");//APN
  delay(1000);
  ShowSerialData();
 
  myserial.println("AT+SAPBR=1,1");
  delay(1000);
  ShowSerialData();
 
  myserial.println("AT+SAPBR=2,1");
  delay(1000);
  ShowSerialData();
 
 
  myserial.println("AT+HTTPINIT");
  delay(1000);
  ShowSerialData();
 
  myserial.println("AT+HTTPPARA=\"CID\",1");
  delay(1000);
  ShowSerialData();
 
  //StaticJsonBuffer<200> jsonBuffer;
  //JsonObject& object = jsonBuffer.createObject();
  DynamicJsonDocument object(1024);

//  object.set("version","CP001/hello.ino.esp32");
  //object.set("deviceId","CP001");
   object["version"] = "POD_003.ino.esp32";
  object["deviceId"] = "CP001";
//  object.printTo(Serial);
  Serial.println(" ");  
  String sendtoserver;
  serializeJson(object, sendtoserver);
    Serial.println(sendtoserver);
  delay(1000);
 
  myserial.println("AT+HTTPPARA=\"URL\",\"https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate\""); //Server address
  delay(1000);
  ShowSerialData();
 
  myserial.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  delay(1000);
  ShowSerialData();
 
 
  myserial.println("AT+HTTPDATA=" + String(sendtoserver.length()) + ",100000");
  Serial.println(sendtoserver);
  delay(1000);
  ShowSerialData();
 
  myserial.println(sendtoserver);
  delay(1000);
  ShowSerialData();

  //AT+HTTPSSL=1
  //HTTPS Request with the SIM800
/*
 * If the requirements on the web server are sufficient, the SIM800 can be used to make an HTTPS request with POST or GET parameters. 
 * To start a request with HTTPS, the URL must start with "https://". With the command [AT+HTTPSSL=1] SSL can be activated. 
 * This command must be sent before [AT+HTTPACTION], i.e. before the actual request is started. 
 * SSL connections to servers with invalid or self-signed certificates can also be performed. 
 * The command [AT+SSLOPT=0,1] disables the verification of SSL certificates.
 */

  myserial.println("AT+HTTPSSL=1");
  delay(1000);
  ShowSerialData();
 
  myserial.println("AT+HTTPACTION=1");
  delay(1000);
  ShowSerialData();
 
  myserial.println("AT+HTTPREAD");
  delay(1000);
  ShowSerialData();
 /* Serial.println(F("Trying OTA"));
  String endpoint = "https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate/POD_003.ino.esp32";
  String ht = "https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate/";
  client.print(String("GET ") + endpoint + " HTTP/1.1\r\n");
  client.print(String("Host: ") + ht + "\r\n");
  client.print("Content-Type: application/octet-stream");
  client.print("Content-Disposition: attachment; filename=POD_003.ino.esp32.bin");//delay(300);
*/

  /*
   * 
   * http.get("/UPDATE.bin");
    http.sendHeader("Content-Type", "application/octet-stream");
    http.sendHeader("Content-Disposition", "attachment; filename=UPDATE.bin");
    http.sendHeader("Connection", "close");
    http.endRequest();
   */
 uint32_t contentLength = knownFileSize;
  File file = SPIFFS.open("/update.bin", FILE_APPEND);
const char server[] = "https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate";
  // if you get a connection, report back via serial:
    if (!client.connect(server, 443))
    {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    Serial.println(" OK");

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


    long timeout = millis();
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

        updateFromFS();
 
  myserial.println("AT+HTTPTERM");
  delay(10000);
  ShowSerialData();
 
  /********************GSM Communication Stops********************/

}

void loop()
{
 // test();
}

void ShowSerialData()
{
  while (myserial.available() != 0)
    Serial.write(myserial.read());
  delay(1000);
 
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
