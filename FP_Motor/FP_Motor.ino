#include "SoftwareSerial.h"       //ESPSoftwareSerial library (download on library manager)
#include "DFRobotDFPlayerMini.h"  //DFRobotDFPlayerMini library (download on library manager)
#include <Adafruit_Fingerprint.h> //Adafruit_Fingerprint library (download on library manager)

#define ON D7       //pin declaration for main power relay
#define STARTER D6  //pin declaration for engine starter relay

bool power;   //variable to store the state of main power
bool engine;  //variable to store the state of engine
unsigned long timer;  //variable to store current millis() counter that will be used for timer

SoftwareSerial fp(D3, D5);  //Software Serial pin declaration for fingerprint sensor (pin D4 was not used because it is a hardware serial TX pin for DFPlayer)
DFRobotDFPlayerMini voice;  //DFPlayermini variable
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fp);  //Fingerprint variable

void setup()
{
  pinMode(ON,OUTPUT);
  pinMode(STARTER,OUTPUT);
  Serial.begin(9600);     //Computer Connection (for development only)
  Serial1.begin(9600);    //DFPlayer Mini (Connect DFPlayer RX pin to D4 pin on NodeMCU)
  voice.begin(Serial1);   //Initialization of DFPlayer on Serial1
  voice.volume(30);       //DFPlayer volume set to 30
  //voice.play(1);
  delay(100);
  Serial.println("\n\nMotorcycle Fingerprint Key");
 
  finger.begin(57600);    //Software Serial initialization for fingerprint
  delay(5);
  /* // Only used for development to check wether fingerprint sensor detected or not
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    setup;
  }
  */
  //Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  //Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  //Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  //Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  //Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  //Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  //Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  //Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  //Serial.println("Waiting for valid finger...");
  digitalWrite(ON, LOW);
  digitalWrite(STARTER, LOW);
  power=false;  //initial state value for main power
  engine=false; //initial state value for engine
}

void loop() {
  int fingerID =-1;   //variable for fingerprintID that stored in the module, -1 means no match
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  fingerID = -1;    //getting fingerprint image from sensor
  uint8_t q = finger.image2Tz();
  if (q != FINGERPRINT_OK)  fingerID = -1;    //converting image
  uint8_t r = finger.fingerSearch();
  if (r != FINGERPRINT_OK)  fingerID = -1;    //matching fingerprint
  else if (r == FINGERPRINT_OK) {
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    fingerID = finger.fingerID;
  }
  //Command when power still off and engine still off
  if (!power && !engine && p==FINGERPRINT_OK){
    //wrong finger
    if (fingerID==-1){
    digitalWrite(ON, LOW);
    digitalWrite(STARTER, LOW);
    power=false;
    voice.play(2); //play voice "You are not authorized to use this vehicle"
    Serial.println("You are not authorized to use this vehicle");
    }
    //correct finger
    else if (fingerID>=0) {
      timer = millis();
      digitalWrite(ON, HIGH);
      power=true;
      engine=false;
      voice.play(1); //play voice "Welcome sir!"
      Serial.println("Welcome sir!");
      delay(1000);
      voice.play(3); //play voice "Please tap your finger again to start the engine"
      Serial.println("Please tap your finger again to start the engine");
    }
  }
  //Command when power is already on but engine still off
  else if (power && !engine) {
    //Correct finger
    if (fingerID>=0 && millis()-timer<=10000){
        voice.play(6); //play voice "Starting the engine"
        Serial.println("Starting the engine");
        delay(2000);
        digitalWrite(STARTER, HIGH);
        delay(3000);
        digitalWrite(STARTER, LOW);
        engine=true;
        power=true;
        Serial.println("TURNED ON");
      }
      //Wrong finger or no finger detected after 10 seconds
      else if (engine==false && millis()-timer>10000) {
        digitalWrite(ON, LOW);
        power=false;
        engine=false;
        voice.play(4); //play voice "No finger detected, vehicle will be turned off"
        Serial.println("No finger detected, vehicle will be turned off");
      }
  }
  //Command when both engine and main power turned on
  else if (power && engine && p==FINGERPRINT_OK){
    //Wrong finger do nothing
    if (fingerID==-1){
    digitalWrite(ON, HIGH);
    digitalWrite(STARTER, LOW);
    engine=true;
    power=true;
    //Serial.println("Still ON");
    }
    //Correct finger turn main power and engine off
    else if (fingerID>=0) {
      digitalWrite(ON, LOW);
      digitalWrite(STARTER, LOW);
      engine=false;
      power=false;
      voice.play(5);  //Play voice "Bye, have a great day"
      Serial.println("Bye, have a great day");
      Serial.println("TURNED OFF");
    }
  }
  delay(50);
}
