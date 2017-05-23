#include "cc1101.h"
#include <advancedSerial.h>
#include "RFReceiver.h"
#include "PowerSocketSend.h"

CC1101 cc1101;
RFReceiver receiver = RFReceiver();
PowerSocketSend pss;


// Handle Serial commands
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

#define LEDOUTPUT 13

void blinker() {
  digitalWrite(LEDOUTPUT, HIGH);
  delay(100);
  digitalWrite(LEDOUTPUT, LOW);
  delay(100);
}

void setup()
{
  Serial.begin(9600);
  aSerial.setPrinter(Serial);
  aSerial.setFilter(Level::vvv);

  //attachInterrupt(0, interuptDetected, CHANGE);
  receiver.setInterrupt(0);
  aSerial.level(Level::vvv).println("Starting CC1101...");


  pinMode(LEDOUTPUT, OUTPUT);
  digitalWrite(LEDOUTPUT, LOW);

  // blink once to signal the setup
  blinker();

  aSerial.level(Level::vvv).println("Initialize and set registers.");


  cc1101.init();

  aSerial.level(Level::vvv).println("Setting PA_TABLE.");
  config2();

  //cc1101.setSyncWord(syncWord, false);
  //cc1101.setCarrierFreq(CFREQ_433);
  cc1101.disableAddressCheck();
  cc1101.setIdleState();
  cc1101.setRxState();
  //cc1101.setTxPowerAmp(PA_LowPower);

  delay(750);

  aSerial.level(Level::vvv).print("CC1101_PARTNUM ");
  aSerial.level(Level::vvv).println(cc1101.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
  aSerial.level(Level::vvv).print("CC1101_VERSION ");
  aSerial.level(Level::vvv).println(cc1101.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
  aSerial.level(Level::vvv).print("CC1101_MARCSTATE ");
  aSerial.level(Level::vvv).println(cc1101.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);



  aSerial.level(Level::vvv).println("device initialized");
  aSerial.level(Level::vvv).println("");
  aSerial.level(Level::vvv).println("Help : For 433Mhz Radio Controlled lights, type #Outlet_number#1_or_0");

  inputString.reserve(50);
}



void loop()
{

  // print the string when a newline arrives:
  if (stringComplete) {
    if (inputString == "10")
      pss.checkAndSend(inputString[0] - '0', inputString[1] - '0');

    if (inputString == "11")
      pss.checkAndSend(inputString[0] - '0', inputString[1] - '0');

    if (inputString == "20")
      pss.checkAndSend(inputString[0] - '0', inputString[1] - '0');

    if (inputString == "21")
      pss.checkAndSend(inputString[0] - '0', inputString[1] - '0');

    if (inputString == "30")
      pss.checkAndSend(inputString[0] - '0', inputString[1] - '0');

    if (inputString == "31")
      pss.checkAndSend(inputString[0] - '0', inputString[1] - '0');

    if (inputString == "40")
      pss.checkAndSend(inputString[0] - '0', inputString[1] - '0');

    if (inputString == "41")
      pss.checkAndSend(inputString[0] - '0', inputString[1] - '0');
    //delay(1000);
    //checkAndSend(1,0);
    //send_data(off1, sizeof(off1));

    //delay(1000);
    //checkAndSend(1,1);
    //delay(1000);
    //checkAndSend(1,0);
    //delay(1000);
    //send_data(on1, sizeof(on1));


    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  //mySerialEvent();




  if (receiver.available())
  {
    String line = "{";
    line = line + receiver.available_proto;
    line = line + ";";
    line = line + receiver.available_temp;
    line = line + ";";
    line = line + receiver.available_hum;
    line = line + "}";

    //test = available_temp + "gfd" + available_proto;
    //test = "{"+available_proto+";"+available_temp+";"+available_hum+"}";
    Serial.println(line);
  }


}


void config2()
{
  byte PA_TABLE[] = {0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  cc1101.writeBurstReg(CC1101_PATABLE, PA_TABLE, 8);
}


void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:

    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\r' | inChar == '\n') {
      Serial.println("end of line");
      stringComplete = true;
    } else
    {
      inputString += inChar;
    }
  }
}

