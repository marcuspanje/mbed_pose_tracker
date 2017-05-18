#include "mbed.h"
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include "Pmt9123.h"

using namespace std;


/** performs power up sequence recommended in 
datasheet */
void Pmt9123::init(int freq, int devAddr) {
    _i2c.frequency(freq);
    dev_addr = devAddr;
    wait_ms(100);
    _nReset = 0;
    wait_us(100);
    _nReset = 1;
    wait_ms(100);
    write(0x41, 0xBA);//enable register write
    wait(0.1);

    char buf;
    read(0x00, &buf);

    //mouse_write(0x11, 0xF0); //disable rest mode

    //read motion registers for first time
    read(0x02, &buf);
    read(0x03, &buf);
    read(0x04, &buf);
    read(0x05, &buf);

    write(0x3A, 0x96); //soft reset
    write(0x41, 0xB5);//disable register write

}


void Pmt9123::write(char addr, char data) {

    char buf[2]; 
    buf[0] = addr;
    buf[1] = data;
    _i2c.write(dev_addr << 1, buf, 2 );

}

void Pmt9123::read(char addr, char* buf) {
    _i2c.write(dev_addr << 1, &addr, 1, true);
    _i2c.read(dev_addr << 1, buf, 1);
}


int get12bitTwoComp(unsigned int i) {
    if ((0x01 & (i >> 11)) == 1)  {
        return -((0x0FFF & (~i)) + 1);
    } else {
        return (int) i;
    }
}

void Pmt9123::read_xy(int& x, int& y, int forceRead) {
    char motion;
    char xl;
    char yl;
    char xyh;
    read(0x02, &motion);
    if ((motion & 0x7F) || forceRead) {
        read(0x03, &xl);
        read(0x04, &yl);
        read(0x05, &xyh);
        unsigned int xu = ((xyh << 4) & 0x0F00) | xl;
        x = get12bitTwoComp(xu);
        unsigned int yu = ((xyh << 8) & 0x0F00) | yl;
        y = get12bitTwoComp(yu);
        
    } else {
        x = 0;
        y = 0;
    }
}
        
bool test_TwoComp() {
    bool b1 = (get12bitTwoComp(0x7FF) == 2047);
    bool b2 = (get12bitTwoComp(0x00) == 0);
    bool b3 = (get12bitTwoComp(0x01) == 1);
    bool b4 = (get12bitTwoComp(0x800) == -2048);
    bool b5 = (get12bitTwoComp(0x801) == -2047);
    return b1 && b2 && b3 && b4 && b5;
}

/** perform test on hardware and math functions */

bool Pmt9123::test() {
  //test product id
    char buf;
    read(0x00, &buf);
    bool b1 = (buf == 0x41);
    bool b2 =  test_TwoComp();
    return b1 && b2;

}

