#include <ArduinoJson.h>
// use nodemcu board version 2.3.0
#include <TimedAction.h>
/*including mfrc522 libraries*/
#include <SPI.h>
#include <MFRC522.h>
//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include <FirebaseArduino.h> 
#include <ESP8266WiFi.h>

/*Credentials to wifi and firbase cloud*/
#define FIREBASE_HOST "smart-inventory-8d2fb-default-rtdb.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "ZUs0FW9Gc4CRJZHttqaQMIn76F1Afmw73T8YhOG0"
#define WIFI_SSID "HUAWEI Y8p"
#define WIFI_PASSWORD "12345678"

#define SS_PIN 15  //D8
#define RST_PIN 2 //D4
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;   
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
int statuss = 0;
int out = 0;
int numofIR=4;
int shelf[] = {16,5,4,0};// choose input pin (for Infrared sensor)  put infrared on D0 and D1 
int active_shelf[]={-1,-1,-1,-1};
byte readBlockData1[18];
byte readBlockData2[18];
byte readBlockData4[18];
byte blockData1 [16] ; 
byte blockData2 [16] ;
byte blockData4 [16] ;
byte bufferLen = 18;
int blockNum1 = 1;  
int blockNum2 = 2; 
int blockNum4 = 4;
void check_shelves(){
  for (int k=0; k<numofIR;k++){
  if (digitalRead(active_shelf[k])==1){
    Serial.println("Index no.= "+String(k));
    Serial.println("Value= "+String(active_shelf[k]));
    Firebase.setString("Reset_Shelf","Shelf"+String(k+1));
    delay (1000);
    Firebase.setString("RST", "reset");
    Serial.print ("You have removed item from Shelf "+String(k+1));
    Firebase.setString("Message","1");
    active_shelf[k]= -1;
    delay (1000);
    Firebase.setString("Message","6");
  }
  }
}
 bool ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{
  /* Authenticating the desired data block for Read access using Key A */
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
     Serial.print("Authentication failed for Read: ");
     //Serial.println(mfrc522.GetStatusCodeName());
     Firebase.setString("Message","2");
     return 1;
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
    //Serial.println(mfrc522.GetStatusCodeName());
    return 1;
    Firebase.setString("Message","2");
  }
  else
  {
    Serial.println("Block was read successfully");  
    return 0;
  }
}
bool WriteDataToBlock(int blockNum, byte blockData[]) 
{
  /* Authenticating the desired data block for write access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 1;
   
  }
  else
  {
    Serial.println("Authentication success");
    
  }

  
  /* Write data to the block */
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 1;
  }
  else
  {
    Serial.println("Data was written into Block successfully");
    return 0;
  }
  
}
TimedAction chkThread = TimedAction(50,check_shelves);

void PrintData(int blockNum, byte readBlockData[]){
   /* Print the data read from block */
   Serial.print("\n");
   Serial.print("Data in Block:");
   Serial.print(blockNum);
   Serial.print(" --> ");
   for (int j=0 ; j<16 ; j++)
   {
     Serial.write(readBlockData[j]);
   }
   Serial.print("\n");
}

void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

   /*Beginning of Wifi and Firebase*/
   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                      
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());                                                     
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  
}
void loop() {
  chkThread.check();
 
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  
  // Look for new cards
 
  
  if(Firebase.getString("Write")=="1"){
      
     String intermediate;
  intermediate =(Firebase.getString("ProductName"));
  intermediate.remove(0,1);
  intermediate.remove(intermediate.length()-1);
  intermediate.getBytes(blockData1,16);
  intermediate =(Firebase.getString("ProductWeight"));
  intermediate.remove(0,1);
  intermediate.remove(intermediate.length()-1);
  intermediate.getBytes(blockData2,16);
  intermediate =(Firebase.getString("ProductSpecs"));
  intermediate.remove(0,1);
  intermediate.remove(intermediate.length()-1);
  intermediate.getBytes(blockData4,16);
     
  /* Print type of card (for example, MIFARE 1K) */

     /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
   if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
  Firebase.setString("Message","2");
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {  Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
    Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
         
   /* Call 'WriteDataToBlock' function, which will write data to the block */
   Serial.print("\n");
   
   Serial.println("Writing to Data Block...");
   bool repeat=0;
   repeat=WriteDataToBlock(blockNum1, blockData1);
   repeat=WriteDataToBlock(blockNum2, blockData2);
   repeat=WriteDataToBlock(blockNum4, blockData4);
   if (repeat==1){
    Firebase.setString("Message","2");
    return;}
   /* Read data from the same block */
   Serial.print("\n");
   Serial.println("Reading from Data Block...");
   repeat=ReadDataFromBlock(blockNum1, readBlockData1);
   repeat=ReadDataFromBlock(blockNum2, readBlockData2);
   repeat=ReadDataFromBlock(blockNum4, readBlockData4);
   if (repeat==1){
    Firebase.setString("Message","2");
    return;}
   /* If you want to print the full memory dump, uncomment the next line */
   //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
   PrintData(blockNum1,readBlockData1);
   PrintData(blockNum2,readBlockData2);
   PrintData(blockNum4,readBlockData4);
   Firebase.setString("Written","Yes");
   Firebase.setString("Message","7");
   Firebase.setString("Write","0");
   (Firebase.getString("ProductName")).getBytes(blockData1,16);
   (Firebase.getString("ProductWeight")).getBytes(blockData2,16);
   (Firebase.getString("ProductSpecs")).getBytes(blockData4,16);
   // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
}
 else{
   /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
   if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
  Firebase.setString("Message","2");
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {chkThread.check();
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
    bool repeat=0;
    repeat=ReadDataFromBlock(blockNum1, readBlockData1);
    repeat=ReadDataFromBlock(blockNum2, readBlockData2);
    repeat=ReadDataFromBlock(blockNum4, readBlockData4);
   if (repeat==1){
    
     // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    Serial.println("Scan card again");
    Firebase.setString("Message","2");
    return;
   }
  bool empty;
  int shelf_empty;
  int i;
  String intermediate="";
  
  for(i=0;i<numofIR;i++){
        empty = digitalRead(shelf[i]);
        if(empty==0){
         continue;
        }
        else if(empty==1){
          Firebase.setString("Shelf","Shelf"+String(i+1));
          
          Serial.print("Put the object in shelf "+String(i+1));
          Firebase.setString("Message","3");
          Serial.println();
          shelf_empty=i;
          break;
        }
   }
   chkThread.check();
   delay(5000);
   if(digitalRead (shelf[shelf_empty])==LOW){
   Serial.println("You have placed in right shelf");
   Firebase.setString("Message","4");
   PrintData(blockNum1,readBlockData1);
   PrintData(blockNum2,readBlockData2);
   PrintData(blockNum4,readBlockData4);
   Firebase.setString("Shelf","Shelf"+String(i+1));
    intermediate=String ((char*)readBlockData1);
    Firebase.setString("Product_Name",intermediate);
    intermediate="";
    
     intermediate=String ((char*)readBlockData2);
    Firebase.setString("Product_Quantity",intermediate);
    intermediate="";
    
     intermediate=String ((char*)readBlockData4);
    Firebase.setString("Product_Specs",intermediate);
    
    delay (1500);
    Firebase.setString("Add","add");
    
    delay(1000);
    active_shelf[shelf_empty]=shelf[shelf_empty];
    Serial.println("Index no.= "+String(shelf_empty));
    Serial.println("Value= "+String(active_shelf[shelf_empty]));
    chkThread.check();
   
   }
   
   else if(digitalRead (shelf[shelf_empty])==HIGH)
   {Serial.print("You have placed in wrong shelf");
   Firebase.setString("Message","5");
   }
   
    statuss = 1;
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    delay (1000);
    Firebase.setString("Message","6");
 }
}

 
