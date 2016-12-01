#include "cc1101.h"

CC1101 cc1101;

// Handle Serial commands
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

byte bit_zero = 0x88;
byte bit_one  = 0xEE;
byte bit_float= 0x8E;
byte bits_sync[]= {0x80,0x00,0x00,0x00};


#define LEDOUTPUT 13

byte counter;
byte b;


void blinker(){
digitalWrite(LEDOUTPUT, HIGH);
delay(100);
digitalWrite(LEDOUTPUT, LOW);
delay(100);
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting CC1101...");


  pinMode(LEDOUTPUT, OUTPUT);
  digitalWrite(LEDOUTPUT, LOW);

  // blink once to signal the setup
  blinker();

  // reset the counter
  counter=0;
  Serial.println("Initialize and set registers.");


  cc1101.init();

  Serial.println("Setting PA_TABLE.");
  config2();

  // cc1101.setSyncWord(syncWord, false);
  cc1101.setCarrierFreq(CFREQ_433);
  cc1101.disableAddressCheck();
  //cc1101.setTxPowerAmp(PA_LowPower);

  delay(750);

  Serial.print("CC1101_PARTNUM ");
  Serial.println(cc1101.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
  Serial.print("CC1101_VERSION ");
  Serial.println(cc1101.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
  Serial.print("CC1101_MARCSTATE ");
  Serial.println(cc1101.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);

  Serial.println("device initialized");
  Serial.println("");
  Serial.println("Help : For 433Mhz Radio Controlled lights, type #Outlet_number#1_or_0");

  inputString.reserve(50);
}

//On envoie fois le code (officiellement, c'est seulement 3 fois d'apres la datasheet PT2262)
void checkAndSend(int dataLine, int onOff)
{
    if (dataLine == 1 && onOff == 1)
    send_code("f0f0f0ffff01"); // A on
    
    if (dataLine == 1 && onOff == 0)
    send_code("f0f0f0ffff10"); // A off
    
    if (dataLine == 2 && onOff == 1)
    send_code("f0f0ff0fff01"); // B on
    
    if (dataLine == 2 && onOff == 0)
    send_code("f0f0ff0fff10"); // B off
    
    if (dataLine == 3 && onOff == 1)
    send_code("f0f0fff0ff01"); // C on
    
    if (dataLine == 3 && onOff == 0)
    send_code("f0f0fff0ff10"); // C off
    
    if (dataLine == 4 && onOff == 1)
    send_code("f0f0ffff0f01"); // D on
    
    if (dataLine == 4 && onOff == 0)
    send_code("f0f0ffff0f10"); // D off
}

//On compose une chaine de 3 codes (code+sync) (officiellement, c'est bien 3 fois d'apres la datasheet PT2262)
void send_code(char seq[])
{  
  byte codeToSend[48];
  
  int i = 0;
  int j = 0;
  int k = 0;
  int l = 0;

for (k = 0; k < 3; k++){

  l=0;
    
    while (l < strlen(seq)){

      if (seq[l] == '0')
      {
      //Serial.print("0");
      codeToSend[i] = bit_zero;
      }
      
      if (seq[l] == '1')
      {
      //Serial.print("1");
      codeToSend[i] = bit_one;
      }
      
      if (seq[l] == 'f')
      {
      //Serial.print("f");
      codeToSend[i] = bit_float;
      }
      

      //Serial.print(codeToSend[i], HEX);
      //Serial.print(",");

      i++;
      l++;
    }
    //Serial.println(i);

    j = 0;
    for(int j=0;j<sizeof(bits_sync);j++)
    {
      
      //Serial.println("Appending Sync");
      
      //Serial.println(j);
      codeToSend[i] = bits_sync[j];
      i++;
    }
}

    //Serial.print("Code à envoyer : ");
    //for(int i = 0; i < sizeof(codeToSend); i++)
  //{
    //Serial.print(codeToSend[i]);

    
    //Serial.print(codeToSend[i],HEX);
    //Serial.print(",");
  //}

//Serial.println("");
send_data(codeToSend,sizeof(codeToSend));
}



void send_data(byte *array, byte nb) {
  Serial.println("Data sending starting");
  CCPACKET data;
  byte blinkCount=counter++;

  data.length = nb;
  for(int i=0;i<data.length;i++) data.data[i] = array[i];

  if(cc1101.sendData(data)){
    Serial.print(blinkCount,HEX);
    Serial.println(" sent ok :)");
    //blinker();
  }else{
    Serial.println("sent failed :(");
  }
}

void loop()
{
  // print the string when a newline arrives:
  if (stringComplete) {
    if(inputString == "10")
      checkAndSend(inputString[0] - '0',inputString[1] - '0');

    if(inputString == "11")
      checkAndSend(inputString[0] - '0',inputString[1] - '0');

    if(inputString == "20")
      checkAndSend(inputString[0] - '0',inputString[1] - '0');

    if(inputString == "21")
      checkAndSend(inputString[0] - '0',inputString[1] - '0');

    if(inputString == "30")
      checkAndSend(inputString[0] - '0',inputString[1] - '0');

    if(inputString == "31")
      checkAndSend(inputString[0] - '0',inputString[1] - '0');

    if(inputString == "40")
      checkAndSend(inputString[0] - '0',inputString[1] - '0');

    if(inputString == "41")
      checkAndSend(inputString[0] - '0',inputString[1] - '0');
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
  mySerialEvent();
}

/*
  SerialEvent doesn't work with rfBee, so we create our function we will call manually
 */
void mySerialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:

    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\r' | inChar == '\n') {
	Serial.println("end of line");
      stringComplete = true;
    }else
    {
          inputString += inChar;
    }
  }
}


void config2()
{
byte PA_TABLE[]= {0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00};
cc1101.writeBurstReg(CC1101_PATABLE,PA_TABLE,8);
}
