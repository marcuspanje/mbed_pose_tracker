#ifndef PMT9123_H
#define PMT9123_H

class Pmt9123 {
  public : 
    Pmt9123(PinName sda, PinName scl, PinName nReset): _i2c(sda, scl), _nReset(nReset) {} ;
    void init(int freq, int devAddr);
    void read_xy(int& x, int& y, int forceRead);
    void read(char addr, char* buf);
    void write(char addr, char data);
    bool test();
    int dev_addr;

  private:
    I2C _i2c;
    DigitalOut _nReset;
    
};
#endif
