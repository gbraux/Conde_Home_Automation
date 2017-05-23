#include "RFReceiver.h"


  bool RFReceiver::syncDetected = false;
  bool RFReceiver::gotPulse = false;
  int RFReceiver::bitcount = 0;
  bool RFReceiver::dataAvailable = 0;
  int RFReceiver::protocolType = 1;
  long RFReceiver::lt = 0;
  int RFReceiver::lastBit = 0;
  bool RFReceiver::datas[50];
  bool RFReceiver::goodDatas[50];
  int RFReceiver::goodProtocol;


void RFReceiver::setInterrupt(int interrupt) {
  attachInterrupt(interrupt, RFReceiver::interuptDetected, CHANGE);
}

RFReceiver::RFReceiver() {
  int a=1;
}

void RFReceiver::interuptDetected(void) {
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

bool RFReceiver::available(void) {
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

void RFReceiver::computeData(void) {

  available_proto = 0;
  available_hum = 0;
  available_temp = 0;
  int hum;
  long temp;
  int channel;
  int battery;
  
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
    int l = 2;

    available_proto = 1;
  
    aSerial.level(Level::vvv).print("\n");
    aSerial.level(Level::vvv).println("Data Protocol 1 received");
    aSerial.level(Level::vvv).print("Got Datas : ");
    for (int i = 0; i < 36; i++)
    {
      aSerial.level(Level::vvv).print(goodDatas[i]);
    }

    //CHANNEL --------------------------------------------------------
    
    aSerial.level(Level::vvv).print("\nCanal : ");
    for (int i = 9; i < 12; i++)
    {
      bitWrite(channel, l, goodDatas[i]);
      //aSerial.level(Level::vvv).print(goodDatas[i]);
      l--;
    }

    aSerial.level(Level::vvv).print(channel);
    aSerial.level(Level::vvv).print("\n");


    //BATTERIE --------------------------------------------------------
    
    aSerial.level(Level::vvv).print("Batterie : ");

    bitWrite(battery, 0, goodDatas[8]);
    
    aSerial.level(Level::vvv).print(battery);
    aSerial.level(Level::vvv).print("\n");


    //HUMIDITE --------------------------------------------------------
    
    aSerial.level(Level::vvv).print("Tx humidite : ");
    for (int i = 28; i < 36; i++)
    {
      bitWrite(hum, j, goodDatas[i]);
      //aSerial.level(Level::vvv).print(goodDatas[i]);
      j--;
    }

    aSerial.level(Level::vvv).print(hum);
    aSerial.level(Level::vvv).print(" %\n");

    //TEMPERATURE ----------------------------------------------------

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

