



class PowerSocketSend {
  public:
    void checkAndSend(int dataLine, int onOff);
  private:
    void send_code(char seq[]);
    void send_data(byte *array, byte nb);

    const byte bit_zero = 0x88;
    const byte bit_one  = 0xEE;
    const byte bit_float= 0x8E;
    const byte bits_sync[4]= {0x80,0x00,0x00,0x00};
    CC1101 cc1101;
};

