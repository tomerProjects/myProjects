#define VERSION "1.1"
#include <WiFi.h>
#include <inttypes.h>
#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include <lm75.h>
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#define AOUT_PIN 34 // ESP32 pin GIOP36 (ADC0) that connects to AOUT pin of moisture sensor

#define WIFI_SSID "KinCollege"
#define WIFI_PASSWORD ""

// Insert Firebase project API Key
#define API_KEY "AIzaSyBZ7qXVKDjFi94uVHB8bgdtd6OCiSkfMgs"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://smartplant-f3e98-default-rtdb.europe-west1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
TempI2C_LM75 termo = TempI2C_LM75(0x48,TempI2C_LM75::nine_bits);
 
void setup()
{
Serial.begin(9600);
Serial.println("Start");
Serial.print("Actual temp ");
Serial.print(termo.getTemp());
Serial.println(" oC");
delay(2000);
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
Serial.print("Connecting to Wi-Fi");
while (WiFi.status() != WL_CONNECTED){
  Serial.print(".");
  delay(300);
}
Serial.println();
Serial.print("Connected with IP: ");
Serial.println(WiFi.localIP());
Serial.println();

/* Assign the api key (required) */
config.api_key = API_KEY;

/* Assign the RTDB URL (required) */
config.database_url = DATABASE_URL;

/* Sign up */
if (Firebase.signUp(&config, &auth, "", "")){
  Serial.println("ok");
  signupOK = true;
}
else{
  Serial.printf("%s\n", config.signer.signupError.message.c_str());
}

/* Assign the callback function for the long running token generation task */
config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

Firebase.begin(&config, &auth);
Firebase.reconnectWiFi(true);
}
 
void loop()
{
int value = analogRead(AOUT_PIN); // read the analog value from sensor
double temp=termo.getTemp();


Serial.print("Moisture value: ");
Serial.println(value);
Serial.print(temp);
Serial.println(" oC");
if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
  if (Firebase.RTDB.setFloat(&fbdo, "test/temp", temp)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "test/moist", value)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }  
delay(5000);

}
