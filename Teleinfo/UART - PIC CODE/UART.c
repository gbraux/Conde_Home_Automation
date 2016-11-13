// ------- FONCTIONS --------
char i, error, byte_read;                 // Auxiliary variables
void UART1_Write_Text_7E1(unsigned char text[]);
void UART1_Write_Text_8N1(unsigned char text[]);
void UART2_Write_Text_7E1(unsigned char text[]);
void UART2_Write_Text_8N1(unsigned char text[]);
char ConvertSerial_8N1_to_7E1(unsigned char serialInput);
char ConvertSerial_7E1_to_8N1(unsigned char serialInput);
unsigned short GetAndForwardTeleinfo(unsigned char n);
unsigned short get7bitsEvenParity(unsigned char n);
void getTrameTeleinfo();
int checkTrameChecksum();
int checkTrameUniteChecksum ();
char* convertCharToString(unsigned char value);
void getTemperature_1wire();
void generateTempUnit();
void generateChecksum();
void ledBlink(int nbBlink);
float getVddVoltage();
void generateVddUnit();

// ------- VARIABLES --------
char ch[2] ;
char car_prec ;
char message[2000] ;
char *q;
char msg_unite[50] ;
int j = 0;
char currentChar;
int checksumResult;
unsigned char sum = 0;
unsigned int nbchar;
unsigned char checksumValue;
unsigned int k = 0;
const unsigned short TEMP_RESOLUTION = 12;
unsigned char temp_unit[30];
unsigned char vdd_unit[30];
char *temp_text = "000.0000";
int sleepCount = 0;
int sendcount = 0;


// ------ Constantes -----
#define trameStartBit 0x02
#define trameEndBit 0x03
#define unitStartBit '\n'
#define unitStopBit '\r'
#define numberToSleep 5  // x 120secondes (postscaler @ max)
#define numberToSend  8

/*#define trameStartBit 'a'
#define trameEndBit 'z'
#define unitStartBit 'o'
#define unitStopBit 'p'*/

// ------- GESTION DU DEBUG -------
//#define DEBUG
#define DEGUG_Continuous_Send
//#define DEBUG_Verbose

void main(){
  
  C1ON_bit = 0;               // Disable comparators
  C2ON_bit = 0;
  
  
  TRISA = 0;           // set direction to be output
  TRISB = 0;
  TRISC = 0;
  
  PORTA = 0;
  PORTB = 0;
  PORTB = 0;
  PORTC = 0;
  


  //On met le XRF en standby
  PORTB.B0 = 1;

  
  //On vérifie que le WDT est bien désactivé
  WDTCON.SWDTEN = 0;

  //Get Temperature
  TRISA.B1 = 1;
  ANSELA.B1 = 0;

  

  
  ledBlink(5);
  
  //On init les UART (1 = XFR / 2 = Teleinfo)
  UART1_Init(1200);
  UART2_Init(1200);
  
  //Init One-Wire
  Delay_1sec();
  getTemperature_1wire();
  Delay_1sec();
  getTemperature_1wire();
  Delay_1sec();
  
  
  ledBlink(5);


  

  while(1) {
    
    message[0]='\0' ;
    
    #ifdef DEGUG_Continuous_Send
      strcat(message, "\nADCO 040726006524 ;\r\nOPTARIF HC.. <\r\nISOUSC 25 =\r\nHCHC 024770643 '\r\nHCHP 056535914 9\r\nPTEC HC.. S\r\nIINST1 000 H\r\nIINST2 001 J\r\nIINST3 001 K\r\nIMAX1 033 6\r\nIMAX2 039 =\r\nIMAX3 030 5\r\nPMAX 17040 2\r\nPAPP 00470 ,\r\nHHPHC D /\r\nMOTDETAT 000000 B\r\nPPOT 00 #\r");
    #endif
    
    #ifdef DEBUG
     //On fait démarer le XRF plus tot pour pouvoir transmettres des infos de débug
     //Power-on XRF
     PORTB.B0 = 0;
     //On laisse XRF démarrer tranquilement
    Delay_ms(1500);
    #endif

       while (!checkTrameChecksum())
       {
        //Acquisition = On allume la led
        PORTA.B3 = 1;
        
             getTrameTeleinfo();
             
        //Fin = On eteinds
        PORTA.B3 = 0;
       }

    //On a une (normalement) trame Teleinfo clean en stock !
    ledBlink(2);

    //On obtient Temperature et Vdd
    getTemperature_1wire();
    generateTempUnit();
    generateVddUnit();
    
    /*#ifdef DEBUG
    UART1_Write_Text_7E1("Trame TI = ");
    UART1_Write_Text_7E1(message);
    UART1_Write_Text_7E1("\r\n");
    UART1_Write_Text_7E1("Global Checksum = ");
    #endif*/
    
    //Power-on XRF
    PORTB.B0 = 0;
    //On laisse XRF démarrer tranquilement
    Delay_ms(1500);
    
    sendcount = 0;
    while(sendcount < numberToSend)
    {
/*#ifdef DEBUG
      UART1_Write_Text_7E1("Global Checksum = ");
      UART1_Write_Text_7E1("OK");
      UART1_Write_Text_7E1("\r\n");
      UART1_Write_Text_7E1("Message envoye au PI = ");
      #endif*/

/*#ifdef DEGUG_Continuous_Send*/
        UART1_Write(ConvertSerial_8N1_to_7E1(trameStartBit));
        UART1_Write_Text_7E1(message);
        UART1_Write_Text_7E1(temp_unit);
        UART1_Write_Text_7E1(vdd_unit);
        UART1_Write(ConvertSerial_8N1_to_7E1(trameEndBit));
        /*Delay_1sec();
#else
        UART1_Write(ConvertSerial_8N1_to_7E1(trameStartBit));
        UART1_Write_Text_7E1(message);
        UART1_Write_Text_7E1(temp_unit);
        UART1_Write_Text_7E1(vdd_unit);
        UART1_Write(ConvertSerial_8N1_to_7E1(trameEndBit));
      #endif*/
      
      
      #ifdef DEBUG
      UART1_Write_Text_7E1("\r\n");
      #endif

    ledBlink(1);
    sendcount++;
    }
    
      //Si on en est là, c'est qu'on a bien envoyé n fois
      //On met le XRF en standby
      PORTB.B0 = 1;
      
      while (sleepCount < numberToSleep)
      {
       #ifdef DEGUG_Continuous_Send
       Delay_1sec();
       #else
       WDTCON.SWDTEN = 1;
       asm { SLEEP }
       WDTCON.SWDTEN = 0;
       #endif
       sleepCount++;
      }
      sleepCount = 0;
      sendcount = 0;
    }
}


// ----- Recupération Température ------
void getTemperature_1wire() {
  const unsigned short RES_SHIFT = TEMP_RESOLUTION - 8;
  char temp_whole;
  unsigned int temp_fraction;
  unsigned temp;
  int isNegative = 0;
  
  temp_text = "000.0000";


  
  Ow_Reset(&PORTA, 1);                         // Onewire reset signal
  Ow_Write(&PORTA, 1, 0xCC);                   // Issue command SKIP_ROM
  Ow_Write(&PORTA, 1, 0x44);                   // Issue command CONVERT_T
  Delay_us(120);
  
  Ow_Reset(&PORTA, 1);
  Ow_Write(&PORTA, 1, 0xCC);                   // Issue command SKIP_ROM
  Ow_Write(&PORTA, 1, 0xBE);                   // Issue command READ_SCRATCHPAD
  Delay_ms(120);

  temp =  Ow_Read(&PORTA, 1);
  temp = (Ow_Read(&PORTA, 1) << 8) + temp;

  // Check if temperature is negative
  if (temp & 0x8000) {
    temp_text[0] = '-';
    temp = ~temp + 1;
    
    #ifdef DEBUG
    UART1_Write_Text_7E1("--> Temperature negative detectee \r\n");
    #endif
    
    isNegative = 1;
  }

  // Extract temp_whole
  temp_whole = temp >> RES_SHIFT ;


  // Convert temp_whole to characters
  if (temp_whole/100)
    temp_text[0] = temp_whole/100  + 48;
  else
    temp_text[0] = '0';

  
  temp_text[1] = (temp_whole/10)%10 + 48;             // Extract tens digit
  temp_text[2] =  temp_whole%10     + 48;             // Extract ones digit

  // Extract temp_fraction and convert it to unsigned int
  temp_fraction  = temp << (4-RES_SHIFT);
  temp_fraction &= 0x000F;
  temp_fraction *= 625;

  // Convert temp_fraction to characters
  temp_text[4] =  temp_fraction/1000    + 48;         // Extract thousands digit
  temp_text[5] = (temp_fraction/100)%10 + 48;         // Extract hundreds digit
  temp_text[6] = (temp_fraction/10)%10  + 48;         // Extract tens digit
  temp_text[7] =  temp_fraction%10      + 48;         // Extract ones digit

  //Hack pour ajouter "-" quand les températures sont négatives. Limite la résolution à -99.9 et +99.9
  if (isNegative)
      temp_text[0] = '-';
}

void generateTempUnit() {
      msg_unite[0] = '\0';
      strcat(msg_unite, "TEMP");
      strcat(msg_unite, convertCharToString(0x20));
      strcat(msg_unite, temp_text);
      strcat(msg_unite, convertCharToString(0x20));
      
      //Calcul Checksum
      generateChecksum();
      
      msg_unite[0] = '\0';
      strcat(msg_unite, convertCharToString(unitStartBit));
      strcat(msg_unite, "TEMP");
      strcat(msg_unite, convertCharToString(0x20));
      strcat(msg_unite, temp_text);
      strcat(msg_unite, convertCharToString(0x20));
      strcat(msg_unite, convertCharToString(sum));
      strcat(msg_unite, convertCharToString(unitStopBit));
      
      strcpy(temp_unit, msg_unite);
}

void generateVddUnit() {

char vdd_string[30];
vdd_string[0] = '\0';

FloatToStr(getVddVoltage(), vdd_string);
 
      msg_unite[0] = '\0';
      strcat(msg_unite, "VDD");
      strcat(msg_unite, convertCharToString(0x20));
      strcat(msg_unite, vdd_string);
      strcat(msg_unite, convertCharToString(0x20));
      //Calcul Checksum
      generateChecksum();

      msg_unite[0] = '\0';
      strcat(msg_unite, convertCharToString(unitStartBit));
      strcat(msg_unite, "VDD");
      strcat(msg_unite, convertCharToString(0x20));
      strcat(msg_unite, vdd_string);
      strcat(msg_unite, convertCharToString(0x20));
      strcat(msg_unite, convertCharToString(sum));
      strcat(msg_unite, convertCharToString(unitStopBit));

      strcpy(vdd_unit, msg_unite);
}

// ----- Recupération Trame Teleinfo ------
void getTrameTeleinfo() {
  #ifdef DEBUG
  UART1_Write_Text_7E1("Start Listenting \r\n");
  #endif
  
  message[0]='\0' ;
  car_prec='\0';
  ch[0] = '\0';
  
  //Lancement du Watchdog (120 secondes)
  WDTCON.SWDTEN = 1;
  

  while ( ! (ch[0] == trameStartBit && car_prec == trameEndBit) )
  {
    car_prec = ch[0] ;
    #ifdef DEBUG_Verbose
    UART1_Write_Text_7E1("Wait UART2 (Teleinfo) input \r\n");
    #endif
    
    while(UART2_Data_Ready() != 1){}
    
    ch[0] = ConvertSerial_7E1_to_8N1(UART2_Read());
/*if (error)                            // If error was detected
    {
      PORTA = error;
      #ifdef DEBUG
      UART1_Write_Text_7E1("Error Read \r\n");
      #endif
    }*/
    
    #ifdef DEBUG_Verbose
    UART1_Write_Text_7E1("Input OK -->");
    UART1_Write(ConvertSerial_8N1_to_7E1(ch[0]));
    UART1_Write_Text_7E1("\r\n");
    #endif
  }
        // Attend code fin suivi de début trame téléinfo .
  //while ( ! (ch[0] == 0x02 && car_prec == 0x03) ) ;        // Attend code fin suivi de début trame téléinfo .

   #ifdef DEBUG_Verbose
  UART1_Write_Text_7E1("Start Detected \r\n");
  #endif

  while(1)
  {
    while(UART2_Data_Ready() != 1){}
    ch[0] = ConvertSerial_7E1_to_8N1(UART2_Read());
    /*if (error)                            // If error was detected
    PORTA = error;*/

    ch[1] ='\0' ;

    if  (ch[0] == trameEndBit)
        break;
    else
    strcat(message, ch) ;
  }
  //while (ch[0] != 0x03) ;                                // Attend code fin trame téléinfo.

  // Arret du Watchdog (Trame Acquise)
  WDTCON.SWDTEN = 0;

  #ifdef DEBUG
  UART1_Write_Text_7E1("Message Acquis \r\n");
  #endif
}


// ------ Checksum ------
int checkTrameChecksum(void) {
  int isOK = 0;
  msg_unite[0] = '\0';
  for (j=0; j<strlen(message); j++)
  {
    currentChar =  message[j];
    #ifdef DEBUG_Verbose
    UART1_Write_Text_7E1("Octet en traitement -->");
    UART1_Write(ConvertSerial_8N1_to_7E1(currentChar));
    UART1_Write_Text_7E1("\r\n");
    #endif

    if (currentChar != unitStartBit && currentChar != unitStopBit)
    {
      char chartostring[2];
      chartostring[0] = currentChar;
      chartostring[1] = '\0';

      strcat(msg_unite, chartostring);
    }

    if (message[j] == unitStopBit)
    {
    #ifdef DEBUG
      UART1_Write_Text_7E1("Message Trouve -->");
      UART1_Write_Text_7E1(msg_unite);
      UART1_Write_Text_7E1("\r\n");
    #endif

      isOK = checkTrameUniteChecksum();
      msg_unite[0] = '\0';
      
      #ifdef DEBUG
      UART1_Write_Text_7E1("Specific Checksum Result -->");
      if (isOK == 1) { UART1_Write_Text_7E1("OK"); }
      if (isOK == 0) { UART1_Write_Text_7E1("ERROR"); }

      UART1_Write_Text_7E1("\r\n");
      #endif
      
      if (isOK == 0)
      {
         return 0;
      }
    }
  }
         
         if (isOK == 0)
         {
         #ifdef DEBUG
         UART1_Write_Text_7E1("Trame Checksum Error\r\n");
         #endif
         return 0;
         }
         else
         {
         #ifdef DEBUG
         UART1_Write_Text_7E1("Trame Checksum OK\r\n");
         #endif
         return 1;
         }
}
int checkTrameUniteChecksum (void) {
  sum = 0 ;    // Somme des codes ASCII du message
  nbchar = strlen(msg_unite);
  checksumValue = msg_unite[nbchar-1];


  k=0;
  for (k=0;k<nbchar-2;k++)
  {
    sum +=  msg_unite[k];
  }

  sum = (sum & 0x3F) + 0x20 ;
  if ( sum == checksumValue) {
    return 1 ;        // Return 1 si checkum ok.*
  }
  return 0;
}

void generateChecksum() {
  sum = 0 ;    // Somme des codes ASCII du message
  nbchar = strlen(msg_unite);
  //checksumValue = msg_unite[nbchar-1];


  k=0;
  for (k=0;k<nbchar-1;k++)
  {
    sum +=  msg_unite[k];
  }

  sum = (sum & 0x3F) + 0x20 ;
}

char *convertCharToString(unsigned char value) {
 static char string[2];
 string[0] = value;
 string[1] = '\0';
 return string;
}

// ------ Serial Conversion ------
char ConvertSerial_8N1_to_7E1(unsigned char serialInput) {
  int n = 7;
  if (get7bitsEvenParity(serialInput) == 0)
  {
    serialInput = serialInput & ~(1<<n) ;   //force a 0
  }
  else
  {
    serialInput = serialInput | (1<<n) ;   //force a 1
  }
  return serialInput;
}
char ConvertSerial_7E1_to_8N1(unsigned char serialInput) {
  int n = 7;
  serialInput = serialInput & ~(1<<n) ;   //force a 0
  return serialInput;
}

unsigned short get7bitsEvenParity(unsigned char n) {
  int x = 7;
  short parity = 0;

  n = n & ~(1<<x) ;   //force le 8eme bit à zéro

  while (n)
  {
    parity = !parity;
    n      = n & (n - 1);
  }
  return parity;
}


// ------ Serial Transmit ------
void UART1_Write_Text_7E1(unsigned char text[]) {

  unsigned int str_len = 0;
  unsigned int i = 0;

  str_len = strlen(text);
  for(i=0;i<str_len;i++) {
    UART1_Write(ConvertSerial_8N1_to_7E1(text[i]));
  }
}

void UART1_Write_Text_8N1(unsigned char text[]) {

  //UART1_Write_Text(text);
  unsigned int str_len = 0;
  unsigned int i = 0;

  str_len = strlen(text);
  for(i=0;i<str_len;i++) {
    UART1_Write(text[i]);
  }
}
void UART2_Write_Text_7E1(unsigned char text[]) {

  unsigned int str_len = 0;
  unsigned int i = 0;

  str_len = strlen(text);
  for(i=0;i<str_len;i++) {
    UART2_Write(ConvertSerial_8N1_to_7E1(text[i]));
  }
}

void UART2_Write_Text_8N1(unsigned char text[]) {

  //UART2_Write_Text(text);
  unsigned int str_len = 0;
  unsigned int i = 0;

  str_len = strlen(text);
  for(i=0;i<str_len;i++) {
    UART2_Write(text[i]);
  }
}


// ---------- DIVERS ----------
void ledBlink(int nbBlink)
{
 int cnt = 0;
 for (cnt = 0;cnt < nbBlink;cnt++)
 {
 PORTA.B3 = 1;
 Delay_ms(100);
 PORTA.B3 = 0;
 Delay_ms(100);
 }
}

float getVddVoltage()
{

// A TRAVAILLER !!!!!!!!!!!!!!!!!!!



int adc_val = 0;
float vdd;
unsigned int temp_res;
char vdd_string[100];


unsigned int ADC_Value, Temp_F;

// Configure FVR to 2.048 V
  FVRCON = 0b11100000 ;


  //GOOD
  //ADCON1 = 0b10001000;
  
  ADCON1 = 0b00001000;
  ADCON2 = 0b10000000;
  


  // Configure ADCON0 for channel AN0
  ADCON0.CHS0 = 0;
  ADCON0.CHS1 = 0;
  ADCON0.CHS2 = 0;
  ADCON0.CHS3 = 0;
  ADCON0.CHS4 = 0;
  ADCON0.ADON = 1;  // enable A/D converter

  ADCON0.F1 = 1;     // start conversion, GO/DONE = 1
  while (ADCON0.F1); // wait for conversion
  
  ADC_Value = (ADRESH << 8) + ADRESL;
  //vdd = ADC_Value / 10000;
  //vdd = (ADC_Value * 2) / 1000 / 2 / 10;
  //vdd = (255 * ADC_Value) / 2.048;
  //vdd = 1.3456;

  vdd = (ADC_Value * 2);
  vdd = vdd / 10000;

  vdd_string[0] = '\0';

  FloatToStr(vdd, vdd_string);
  

  
  //float_to_string(vdd, vdd_string);
  
  UART1_Write_Text_7E1("ADC Value : ");
  UART1_Write_Text_7E1(vdd_string);
  UART1_Write_Text_7E1("\r\n");
  
  UART1_Write_Text_7E1("DATA Voltage : ");
  UART1_Write_Text_7E1(vdd_string);
  UART1_Write_Text_7E1("\r\n");

  return vdd;

//Test ACD
/*
adc_val = (1024.0 / ADC_Read(0)) * 1.024;
adc_val = (ADRESH << 8);    // Store the result in adc_val
adc_val |= ADRESL;
return vdd = (1024.0 / adc_val) * 1.024;
*/

 /*
  TRISA.B0 = 1;
  ANSELA.B0 = 1;

      VREFCON0 = 0b10010000;
         while (!VREFCON0.FVRST);

   ADCON1 = 0b10000000;
   ADCON2 = 0b10111000;
   ADCON0 = 0b00000001;

   adc_val = 0;
   vdd = 0;

   ADCON0.GO = 1;          // Start a conversion
   while (ADCON0.GO);   // Wait for it to be completed
   adc_val = (ADRESH << 8);    // Store the result in adc_val
   adc_val |= ADRESL;
   return vdd = (1024.0 / adc_val) * 1.024;
   
  */


// The Good One
/*

int adc_val = 0;
float vdd;

      VREFCON0 = 0b10010000;
         while (!VREFCON0.FVRST);

   ADCON1 = 0b10000000;
   ADCON2 = 0b10111000;
   ADCON0 = 0b01111101;

   adc_val = 0;
   vdd = 0;

   ADCON0.GO = 1;          // Start a conversion
   while (ADCON0.GO);   // Wait for it to be completed
   adc_val = (ADRESH << 8);    // Store the result in adc_val
   adc_val |= ADRESL;
   return vdd = (1024.0 / adc_val) * 1.024;

*/
}

































