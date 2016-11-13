#line 1 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"

char i, error, byte_read;
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
#line 61 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
void main(){

 C1ON_bit = 0;
 C2ON_bit = 0;


 TRISA = 0;
 TRISB = 0;
 TRISC = 0;

 PORTA = 0;
 PORTB = 0;
 PORTB = 0;
 PORTC = 0;




 PORTB.B0 = 1;



 WDTCON.SWDTEN = 0;


 TRISA.B1 = 1;
 ANSELA.B1 = 0;




 ledBlink(5);


 UART1_Init(1200);
 UART2_Init(1200);


 Delay_1sec();
 getTemperature_1wire();
 Delay_1sec();
 getTemperature_1wire();
 Delay_1sec();


 ledBlink(5);




 while(1) {

 message[0]='\0' ;


 strcat(message, "\nADCO 040726006524 ;\r\nOPTARIF HC.. <\r\nISOUSC 25 =\r\nHCHC 024770643 '\r\nHCHP 056535914 9\r\nPTEC HC.. S\r\nIINST1 000 H\r\nIINST2 001 J\r\nIINST3 001 K\r\nIMAX1 033 6\r\nIMAX2 039 =\r\nIMAX3 030 5\r\nPMAX 17040 2\r\nPAPP 00470 ,\r\nHHPHC D /\r\nMOTDETAT 000000 B\r\nPPOT 00 #\r");
#line 127 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 while (!checkTrameChecksum())
 {

 PORTA.B3 = 1;

 getTrameTeleinfo();


 PORTA.B3 = 0;
 }


 ledBlink(2);


 getTemperature_1wire();
 generateTempUnit();
 generateVddUnit();
#line 154 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 PORTB.B0 = 0;

 Delay_ms(1500);

 sendcount = 0;
 while(sendcount <  8 )
 {
#line 169 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 UART1_Write(ConvertSerial_8N1_to_7E1( 0x02 ));
 UART1_Write_Text_7E1(message);
 UART1_Write_Text_7E1(temp_unit);
 UART1_Write_Text_7E1(vdd_unit);
 UART1_Write(ConvertSerial_8N1_to_7E1( 0x03 ));
#line 188 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 ledBlink(1);
 sendcount++;
 }



 PORTB.B0 = 1;

 while (sleepCount <  5 )
 {

 Delay_1sec();
#line 205 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 sleepCount++;
 }
 sleepCount = 0;
 sendcount = 0;
 }
}



void getTemperature_1wire() {
 const unsigned short RES_SHIFT = TEMP_RESOLUTION - 8;
 char temp_whole;
 unsigned int temp_fraction;
 unsigned temp;
 int isNegative = 0;

 temp_text = "000.0000";



 Ow_Reset(&PORTA, 1);
 Ow_Write(&PORTA, 1, 0xCC);
 Ow_Write(&PORTA, 1, 0x44);
 Delay_us(120);

 Ow_Reset(&PORTA, 1);
 Ow_Write(&PORTA, 1, 0xCC);
 Ow_Write(&PORTA, 1, 0xBE);
 Delay_ms(120);

 temp = Ow_Read(&PORTA, 1);
 temp = (Ow_Read(&PORTA, 1) << 8) + temp;


 if (temp & 0x8000) {
 temp_text[0] = '-';
 temp = ~temp + 1;
#line 247 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 isNegative = 1;
 }


 temp_whole = temp >> RES_SHIFT ;



 if (temp_whole/100)
 temp_text[0] = temp_whole/100 + 48;
 else
 temp_text[0] = '0';


 temp_text[1] = (temp_whole/10)%10 + 48;
 temp_text[2] = temp_whole%10 + 48;


 temp_fraction = temp << (4-RES_SHIFT);
 temp_fraction &= 0x000F;
 temp_fraction *= 625;


 temp_text[4] = temp_fraction/1000 + 48;
 temp_text[5] = (temp_fraction/100)%10 + 48;
 temp_text[6] = (temp_fraction/10)%10 + 48;
 temp_text[7] = temp_fraction%10 + 48;


 if (isNegative)
 temp_text[0] = '-';
}

void generateTempUnit() {
 msg_unite[0] = '\0';
 strcat(msg_unite, "TEMP");
 strcat(msg_unite, convertCharToString(0x20));
 strcat(msg_unite, temp_text);
 strcat(msg_unite, convertCharToString(0x20));


 generateChecksum();

 msg_unite[0] = '\0';
 strcat(msg_unite, convertCharToString( '\n' ));
 strcat(msg_unite, "TEMP");
 strcat(msg_unite, convertCharToString(0x20));
 strcat(msg_unite, temp_text);
 strcat(msg_unite, convertCharToString(0x20));
 strcat(msg_unite, convertCharToString(sum));
 strcat(msg_unite, convertCharToString( '\r' ));

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

 generateChecksum();

 msg_unite[0] = '\0';
 strcat(msg_unite, convertCharToString( '\n' ));
 strcat(msg_unite, "VDD");
 strcat(msg_unite, convertCharToString(0x20));
 strcat(msg_unite, vdd_string);
 strcat(msg_unite, convertCharToString(0x20));
 strcat(msg_unite, convertCharToString(sum));
 strcat(msg_unite, convertCharToString( '\r' ));

 strcpy(vdd_unit, msg_unite);
}


void getTrameTeleinfo() {
#line 335 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 message[0]='\0' ;
 car_prec='\0';
 ch[0] = '\0';


 WDTCON.SWDTEN = 1;


 while ( ! (ch[0] ==  0x02  && car_prec ==  0x03 ) )
 {
 car_prec = ch[0] ;
#line 350 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 while(UART2_Data_Ready() != 1){}

 ch[0] = ConvertSerial_7E1_to_8N1(UART2_Read());
#line 366 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 }
#line 374 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 while(1)
 {
 while(UART2_Data_Ready() != 1){}
 ch[0] = ConvertSerial_7E1_to_8N1(UART2_Read());
#line 381 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 ch[1] ='\0' ;

 if (ch[0] ==  0x03 )
 break;
 else
 strcat(message, ch) ;
 }



 WDTCON.SWDTEN = 0;
#line 396 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
}



int checkTrameChecksum(void) {
 int isOK = 0;
 msg_unite[0] = '\0';
 for (j=0; j<strlen(message); j++)
 {
 currentChar = message[j];
#line 412 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 if (currentChar !=  '\n'  && currentChar !=  '\r' )
 {
 char chartostring[2];
 chartostring[0] = currentChar;
 chartostring[1] = '\0';

 strcat(msg_unite, chartostring);
 }

 if (message[j] ==  '\r' )
 {
#line 429 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 isOK = checkTrameUniteChecksum();
 msg_unite[0] = '\0';
#line 440 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 if (isOK == 0)
 {
 return 0;
 }
 }
 }

 if (isOK == 0)
 {
#line 452 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 return 0;
 }
 else
 {
#line 459 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
 return 1;
 }
}
int checkTrameUniteChecksum (void) {
 sum = 0 ;
 nbchar = strlen(msg_unite);
 checksumValue = msg_unite[nbchar-1];


 k=0;
 for (k=0;k<nbchar-2;k++)
 {
 sum += msg_unite[k];
 }

 sum = (sum & 0x3F) + 0x20 ;
 if ( sum == checksumValue) {
 return 1 ;
 }
 return 0;
}

void generateChecksum() {
 sum = 0 ;
 nbchar = strlen(msg_unite);



 k=0;
 for (k=0;k<nbchar-1;k++)
 {
 sum += msg_unite[k];
 }

 sum = (sum & 0x3F) + 0x20 ;
}

char *convertCharToString(unsigned char value) {
 static char string[2];
 string[0] = value;
 string[1] = '\0';
 return string;
}


char ConvertSerial_8N1_to_7E1(unsigned char serialInput) {
 int n = 7;
 if (get7bitsEvenParity(serialInput) == 0)
 {
 serialInput = serialInput & ~(1<<n) ;
 }
 else
 {
 serialInput = serialInput | (1<<n) ;
 }
 return serialInput;
}
char ConvertSerial_7E1_to_8N1(unsigned char serialInput) {
 int n = 7;
 serialInput = serialInput & ~(1<<n) ;
 return serialInput;
}

unsigned short get7bitsEvenParity(unsigned char n) {
 int x = 7;
 short parity = 0;

 n = n & ~(1<<x) ;

 while (n)
 {
 parity = !parity;
 n = n & (n - 1);
 }
 return parity;
}



void UART1_Write_Text_7E1(unsigned char text[]) {

 unsigned int str_len = 0;
 unsigned int i = 0;

 str_len = strlen(text);
 for(i=0;i<str_len;i++) {
 UART1_Write(ConvertSerial_8N1_to_7E1(text[i]));
 }
}

void UART1_Write_Text_8N1(unsigned char text[]) {


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


 unsigned int str_len = 0;
 unsigned int i = 0;

 str_len = strlen(text);
 for(i=0;i<str_len;i++) {
 UART2_Write(text[i]);
 }
}



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





int adc_val = 0;
float vdd;
unsigned int temp_res;
char vdd_string[100];


unsigned int ADC_Value, Temp_F;


 FVRCON = 0b11100000 ;





 ADCON1 = 0b00001000;
 ADCON2 = 0b10000000;




 ADCON0.CHS0 = 0;
 ADCON0.CHS1 = 0;
 ADCON0.CHS2 = 0;
 ADCON0.CHS3 = 0;
 ADCON0.CHS4 = 0;
 ADCON0.ADON = 1;

 ADCON0.F1 = 1;
 while (ADCON0.F1);

 ADC_Value = (ADRESH << 8) + ADRESL;





 vdd = (ADC_Value * 2);
 vdd = vdd / 10000;

 vdd_string[0] = '\0';

 FloatToStr(vdd, vdd_string);





 UART1_Write_Text_7E1("ADC Value : ");
 UART1_Write_Text_7E1(vdd_string);
 UART1_Write_Text_7E1("\r\n");

 UART1_Write_Text_7E1("DATA Voltage : ");
 UART1_Write_Text_7E1(vdd_string);
 UART1_Write_Text_7E1("\r\n");

 return vdd;
#line 716 "C:/Users/gubraux/Documents/Perso - Admin/Electronique/EASYPIC/Projets/UART/UART.c"
}
