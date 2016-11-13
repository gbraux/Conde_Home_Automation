#include <wiringPi.h>
#include <sched.h>

void send_Zero();
void send_One();
void send_Sync();
void send_code(char seq[]);
void send_Float();
void Delay_us(int del);
void Delay_ms(int del);

const int short_lenght = 133; // Officialy, it should be 400 
const int long_lenght =    400; // Officialy, it should be 1200
int i = 0;

void main(int argc, char *argv[]) {

  wiringPiSetup () ;
  pinMode (0, OUTPUT) ;
  
  
  if (strcmp("demo", argv[1]) == 0)
  {
    printf("Demo Mode ! \n");
    while(1) {

      checkAndSend(1,1);
      Delay_ms(250);
      checkAndSend(1,0);
      Delay_ms(250);
      checkAndSend(2,1);
      Delay_ms(250);
      checkAndSend(2,0);
      Delay_ms(250);
      checkAndSend(3,1);
      Delay_ms(250);
      checkAndSend(3,0);
      Delay_ms(250);
      checkAndSend(4,1);
      Delay_ms(250);
      checkAndSend(4,0);
      Delay_ms(250);
    }
  }
  else
  {
    checkAndSend(atoi(argv[1]), atoi(argv[2]));
    printf("Canal %u / Commande %u OK \r\n", atoi(argv[1]), atoi(argv[2]));
  }
}

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

//On envoie 30 fois le code (officiellement, c'est seulement 3 fois d'apres la datasheet PT2262)
void send_code(char seq[])
{
  int i = 0;
  int j = 0;

  //scheduler_realtime();
  for (j = 0; j < 30 ; j++)
  {
    for (i = 0; i < strlen(seq); i++){

      if (seq[i] == '0')
      send_Zero();
      if (seq[i] == '1')
      send_One();
      if (seq[i] == 'f')
      send_Float();
    }
    send_Sync();
  }
  //scheduler_standard();
}

void send_Zero()
{
  digitalWrite (0, HIGH);
  Delay_us(short_lenght);
  digitalWrite (0, LOW);
  Delay_us(long_lenght);
  digitalWrite (0, HIGH);
  Delay_us(short_lenght);
  digitalWrite (0, LOW);
  Delay_us(long_lenght);
}

void send_One()
{
  digitalWrite (0, HIGH);
  Delay_us(long_lenght);
  digitalWrite (0, LOW);
  Delay_us(short_lenght);
  digitalWrite (0, HIGH);
  Delay_us(long_lenght);
  digitalWrite (0, LOW);
  Delay_us(short_lenght);
}

void send_Float()
{
  digitalWrite (0, HIGH);
  Delay_us(short_lenght);
  digitalWrite (0, LOW);
  Delay_us(long_lenght);
  digitalWrite (0, HIGH);
  Delay_us(long_lenght);
  digitalWrite (0, LOW);
  Delay_us(short_lenght);
}

void send_Sync()
{
  digitalWrite (0, HIGH);
  Delay_us(short_lenght);
  digitalWrite (0, LOW);
  Delay_us(4133); // Officialy, it should be 12400
}

void Delay_us(int del)
{
  delayMicroseconds(del);
}

void Delay_ms(int del)
{
  delay(del);
}

void scheduler_realtime() {
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if( sched_setscheduler( 0, SCHED_RR, &p ) == -1 ) {
		perror("Failed to switch to realtime scheduler.");
	}
}
 
void scheduler_standard() {
	struct sched_param p;
	p.__sched_priority = 0;
	if( sched_setscheduler( 0, SCHED_OTHER, &p ) == -1 ) {
		perror("Failed to switch to normal scheduler.");
	}
}