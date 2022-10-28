#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>
  
const char* ssid = "arvind197";                     //<-----   change this 
const char* password =  "alok@1234";           // <-----   change this
#define RST_PIN  22     // Configurable, see typical pin layout above
#define SS_PIN   21     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;  
//ESP8266WiFiMulti WiFiMulti;
MFRC522::StatusCode status;      

/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  

/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
  
byte readBlockData[18];

void setup() {
  Serial.begin(9600);
  SPI.begin();
  delay(4000);                                             //Delay needed before calling the WiFi.begin
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {                  //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
   mfrc522.PCD_Init();
  Serial.println("Scan a MIFARE 1K Tag to write data...");
}
  
void loop() {

    /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);
  /* If you want to print the full memory dump, uncomment the next line */
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  
  /* Print the data read from block */
  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  for (int j=0 ; j<16 ; j++)
  {
    Serial.write(readBlockData[j]);
  }
  Serial.println(String((char*)readBlockData));
 
 if(WiFi.status()== WL_CONNECTED){                           //Check WiFi connection status
   HTTPClient http;   
   http.begin("https://script.google.com/macros/s/AKfycbxsCku81etlZt64USfRbaNYGitLx17mNKqqWISpLwShz7Ww5orfdnxT-l2yenFv71ls/exec");  //Specify destination for HTTP request
   http.addHeader("Content-Type", "application/json");             //Specify content-type header
   
    String str = String((char*)readBlockData);
    
    StaticJsonDocument<200> doc;
   doc["name"] = str;
   JsonArray data = doc.createNestedArray("data");
   data.add(48.756080);
   String requestBody;
   serializeJson(doc, requestBody);
   Serial.println(requestBody);
   
   int httpResponseCode = http.POST(requestBody) ;   //Send the actual POST request
   
   if(httpResponseCode>0){
    String response = http.getString();                       //Get the response to the request
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
   }else{
  
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  
   }
  
   http.end();  //Free resources
  
 }
 else{
  
    Serial.println("Error in WiFi connection");   
  
 }
  
  delay(1000);  //Send a request every 10 seconds
}


void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
  else
  {
    Serial.println("Authentication success");
  }

  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Block was read successfully");  
  }
}
