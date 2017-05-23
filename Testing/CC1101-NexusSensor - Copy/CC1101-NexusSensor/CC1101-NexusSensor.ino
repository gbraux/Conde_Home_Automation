#include "cc1101.h"
#include <advancedSerial.h>


CC1101 cc1101;

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
  aSerial.setFilter(Level::vv);

  attachInterrupt(0, interuptDetected, CHANGE);
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






//int laststate = HIGH;
//int gotAStart = 0;
//int freq = 0x0;
//double intf = 433;
//long cnt = 0;
long lt = 0;
int lastBit = 0;
//int bf;
//int found2k;

int available_proto;
int available_hum;
float available_temp;



void loop()
{

  if (available())
  {
    String line = "{";
    line = line + available_proto;
    line = line + ";";
    line = line + available_temp;
    line = line + ";";
    line = line + available_hum;
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


//bool startFound;
bool syncDetected = false;
bool gotPulse = false;
int bitcount = 0;
bool datas[50];
bool dataAvailable = 0;
int protocolType = 1;
bool goodDatas[50];
int goodProtocol;

void interuptDetected(void) {
  long ct = micros();

  lastBit = ct - lt;

  if ((ct - lt > 3900) && (ct - lt < 4100) && (syncDetected == false)) {
    syncDetected = true;
    protocolType = 1;
    //aSerial.level(Level::vvv).println("Sync Found");
    lt = ct;
    return;
  }

  if ((ct - lt > 7850) && (ct - lt < 8050) && (syncDetected == false)) {
    syncDetected = true;
    protocolType = 2;
    //aSerial.level(Level::vvv).println("Sync Found New Protocol");
    //aSerial.level(Level::vvv).println(ct-lt);
    lt = ct;
    return;
  }


  if ((syncDetected) && (protocolType == 1))
  {
    if (gotPulse)
    {
      if ((ct - lt > 1896) && (ct - lt < 2086)) {
        //aSerial.level(Level::vvv).println("Bit = 1");
        datas[bitcount] = 1;
        gotPulse = false;
        bitcount++;
      }
      else if ((ct - lt > 893) && (ct - lt < 1093)) {
        //aSerial.level(Level::vvv).println("Bit = 0");
        datas[bitcount] = 0;
        gotPulse = false;
        bitcount++;
      }
      else {
        //aSerial.level(Level::vvv).println("Got some data count = ");
        //aSerial.level(Level::vvv).println(bitcount);
        syncDetected = false;
        gotPulse = false;
        bitcount = 0;
      }
    }
    else if ((ct - lt > 384) && (ct - lt < 584))
    {
      //aSerial.level(Level::vvv).println("Pulse Detected");
      //gotPulse = true;
      //syncDetected = false;
      gotPulse = true;
    }
  }



  if ((syncDetected) && (protocolType == 2))
  {

    //aSerial.level(Level::vvv).println(bitcount);
    if (gotPulse)
    {

      //aSerial.level(Level::vvv).println(ct-lt);
      if ((ct - lt > 3000) && (ct - lt < 4200)) {
        //aSerial.level(Level::vvv).println("Bit = 1");
        //aSerial.level(Level::vvv).println(ct-lt);
        datas[bitcount] = 1;
        gotPulse = false;
        bitcount++;
      }
      else if ((ct - lt > 1500) && (ct - lt < 2100)) {
        //aSerial.level(Level::vvv).println("Bit = 0");
        //aSerial.level(Level::vvv).println(ct-lt);
        datas[bitcount] = 0;
        gotPulse = false;
        bitcount++;
      }
      else {
        //aSerial.level(Level::vvv).println("Got some data count = ");
        //aSerial.level(Level::vvv).print("\nMissed --> ");
        //aSerial.level(Level::vvv).println(ct-lt);
        //aSerial.level(Level::vvv).println(bitcount);
        syncDetected = false;
        gotPulse = false;
        bitcount = 0;
      }
    }
    else if ((ct - lt > 400) && (ct - lt < 700))
      //else if (true)
    {
      //aSerial.level(Level::vvv).println("Pulse Detected");
      //aSerial.level(Level::vvv).println(ct-lt);
      //gotPulse = true;
      //syncDetected = false;

      gotPulse = true;
    }
    //else if (bitcount > 0)
    //{
    //aSerial.level(Level::vvv).print("\nMissed Pulse --> ");
    //aSerial.level(Level::vvv).println(ct-lt);
    //aSerial.level(Level::vvv).println(bitcount);

    //}

  }

  //aSerial.level(Level::vvv).println(bitcount);

  if ((bitcount == 36) && (protocolType == 1))
  {
    //aSerial.level(Level::vvv).println("Successfull Transmission (Protocol 1) !");
    memcpy(goodDatas, datas, sizeof(datas));
    goodProtocol = 1;
    //goodDatas = datas;
    dataAvailable = true;
  }

  if ((bitcount == 40) && (protocolType == 2))
  {
    //aSerial.level(Level::vvv).println("Successfull Transmission (Protocol 2) !");
    memcpy(goodDatas, datas, sizeof(datas));
    goodProtocol = 2;
    //goodDatas = datas;
    dataAvailable = true;
  }



  //for(int i = 0; i < 36; i++)
  //{
  //  aSerial.level(Level::vvv).print(datas[i]);
  //}


  lt = ct;
}

bool available(void) {
  if (dataAvailable)
  {
    computeData();
    dataAvailable = false;
    return true;
  }
  else
  {
    return false;
  }
}

void computeData(void) {

  available_proto = 0;
  available_hum = 0;
  available_temp = 0;
  int hum;
  long temp;
  
  if (goodProtocol == 2)
  {
    hum = 0;
    int humU = 0;
    int humD = 0;
    int j = 3;
    int k = 11;

    available_proto = 2;

    aSerial.level(Level::vvv).print("\n");
    aSerial.level(Level::vvv).println("Data Protocol 2 received");
    aSerial.level(Level::vvv).print("Got Datas : ");
    for (int i = 0; i < 40; i++)
    {
      aSerial.level(Level::vvv).print(goodDatas[i]);
    }

    aSerial.level(Level::vvv).print("\nTx humidite : ");
    for (int i = 32; i < 36; i++)
    {
      bitWrite(humU, j, goodDatas[i]);
      //aSerial.level(Level::vvv).print(goodDatas[i]);
      j--;
    }

    j = 3;
    for (int i = 28; i < 32; i++)
    {
      bitWrite(humD, j, goodDatas[i]);
      //aSerial.level(Level::vvv).print(goodDatas[i]);
      j--;
    }

    hum = (((int)humD * 10) + (int)humU);
    aSerial.level(Level::vvv).print(hum);
    aSerial.level(Level::vvv).print(" %\n");



    aSerial.level(Level::vvv).print("Temperature Binary : ");


    for (int i = 16; i < 28; i++)
    {
      bitWrite(temp, k, goodDatas[i]);
      aSerial.level(Level::vvv).print(goodDatas[i]);
      k--;
    }

    aSerial.level(Level::vvv).print("\nTemperature Before Conversion to Float : ");
    aSerial.level(Level::vvv).print(temp);

    float floatTemp = temp;
    floatTemp = (0.05 * floatTemp) - 60;
    aSerial.level(Level::vvv).print("\nTemperature : ");
    aSerial.level(Level::vvv).print(floatTemp);
    aSerial.level(Level::vvv).print(" degC\n");



    available_hum = hum;
    available_temp = floatTemp;

  }


  if (goodProtocol == 1)
  {
    hum = 0;
    temp = 0;
    int j = 7;
    int k = 11;

    available_proto = 1;

    aSerial.level(Level::vvv).print("\n");
    aSerial.level(Level::vvv).println("Data Protocol 2 received");
    aSerial.level(Level::vvv).print("Got Datas : ");
    for (int i = 0; i < 36; i++)
    {
      aSerial.level(Level::vvv).print(goodDatas[i]);
    }

    aSerial.level(Level::vvv).print("\nTx humidite : ");
    for (int i = 28; i < 36; i++)
    {
      bitWrite(hum, j, goodDatas[i]);
      //aSerial.level(Level::vvv).print(goodDatas[i]);
      j--;
    }

    aSerial.level(Level::vvv).print(hum);
    aSerial.level(Level::vvv).print(" %\n");

    aSerial.level(Level::vvv).print("Temperature Binary : ");

    if (goodDatas[12] == 1)
    {
      temp = ~temp;
    }

    for (int i = 12; i < 24; i++)
    {
      bitWrite(temp, k, goodDatas[i]);
      aSerial.level(Level::vvv).print(goodDatas[i]);
      k--;
    }

    aSerial.level(Level::vvv).print("\nTemperature Before Conversion to Float : ");
    aSerial.level(Level::vvv).print(temp);

    float floatTemp = temp;
    floatTemp = floatTemp / 10;
    aSerial.level(Level::vvv).print("\nTemperature : ");
    aSerial.level(Level::vvv).print(floatTemp);
    aSerial.level(Level::vvv).print(" degC\n");


    available_hum = hum;
    available_temp = floatTemp;
  }
}

