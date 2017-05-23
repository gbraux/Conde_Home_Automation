#include "Arduino.h"
#include <advancedSerial.h>

class RFReceiver
{
  public:
    RFReceiver();
    void setInterrupt(int interrupt);
    bool available(void);
    int available_proto;
    int available_hum;
    float available_temp;

  private:
    static void interuptDetected(void);
    void computeData(void);
    
    static bool syncDetected;
    static bool gotPulse;
    static int bitcount;
    static bool datas[50];
    static bool dataAvailable;
    static int protocolType;
    static bool goodDatas[50];
    static int goodProtocol;
    static long lt;
    static int lastBit;

};

