#include "Arduino.h"
#include "cc1101.h"
#include "PowerSocketSend.h"







//On envoie fois le code (officiellement, c'est seulement 3 fois d'apres la datasheet PT2262)
void PowerSocketSend::checkAndSend(int dataLine, int onOff)
{
  Serial.println("PSS CHECK SEND");
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
void PowerSocketSend::send_code(char seq[])
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



void PowerSocketSend::send_data(byte *array, byte nb) {
  Serial.println("Data sending starting");
  CCPACKET data;
  //byte blinkCount=counter++;

  data.length = nb;
  for(int i=0;i<data.length;i++) data.data[i] = array[i];

  if(cc1101.sendData(data)){
    //Serial.print(blinkCount,HEX);
    Serial.println(" sent ok :)");
    //blinker();
  }else{
    Serial.println("sent failed :(");
  }
}
