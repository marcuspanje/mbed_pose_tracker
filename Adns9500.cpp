#include "mbed.h"
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include "Adns9500.h"

using namespace std;

/**
This program publishes the pose of a moving object over
a serial connection.
At the start of a  specified period, two independent, random axes (x,y,z) 
are selected  for translation/rotation, and the probe moves along/around them 
*/

const double M_PI1 = 3.14159265359;

//SPI spi(p5, p6, p7);
//DigitalOut ncs(p16);
const int tsrad_us = 100; //delay between address read and data out
const int tsrw_us = 20; // delay between data out and subsequent read

//constructor
Adns9500::Adns9500(PinName mosi, PinName miso, PinName sclk, PinName ncs): _spi(mosi, miso, sclk), _ncs(ncs) {
    //_spi(0,0,0);
    //_ncs(0);
}
      
double wrapToPi(double theta) {
    double ret;
    if (theta > 0) {
      ret = fmod(theta + M_PI1, 2.0*M_PI1) - M_PI1;  
    } else {
      ret = fmod(theta - M_PI1, 2.0*M_PI1) + M_PI1;  
    }
    return ret;
}

bool compareFloat(double f1, double f2, double eps) {
    return (fabsf(f1-f2) < eps);
}

void test_wrapToPi() {
    double eps = 0.1; 
    bool b1 = compareFloat(wrapToPi(M_PI1-0.1), M_PI1-0.1, eps);
    bool b2 = compareFloat(wrapToPi(M_PI1+0.1), -(M_PI1-0.1), eps);
    bool b3 = compareFloat(wrapToPi(-(M_PI1-0.1)), -(M_PI1-0.1), eps);
    bool b4 = compareFloat(wrapToPi(-(M_PI1+0.1)), M_PI1-0.1, eps);
    bool b5 = compareFloat(wrapToPi(2*M_PI1-0.1), -0.1, eps);
    bool b6 = compareFloat(wrapToPi(2*M_PI1+0.1), 0.1, eps);
    bool b7 = compareFloat(wrapToPi(-(2*M_PI1-0.1)), 0.1, eps);
    bool b8 = compareFloat(wrapToPi(-(2*M_PI1+0.1)), -0.1, eps);
    bool result = b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8;
}



char Adns9500::spi_read(int addr) {
    _ncs = 0;
    _spi.write(addr);
    wait_us(tsrad_us);
    int data = _spi.write(0x00);
    _ncs = 1;
    wait_us(tsrw_us);
    return (data & 0xFF);
}

int getTwoComplementInt(char hiByte, char loByte) {
    unsigned int i = (hiByte << 8) | (0x00FF & loByte);
    int i2c;
    if (hiByte & 0x80) {
        i2c = -((0xFFFF&(~i)) + 1);
    } else {
        i2c = i;
    }
    return i2c;
}

void Adns9500::spi_read_xy(int &x, int &y) {
    char xl = spi_read(0x03);
    char xh = spi_read(0x04);
    char yl = spi_read(0x05);
    char yh = spi_read(0x06);
    x = getTwoComplementInt(xh, xl);
    y = getTwoComplementInt(yh, yl);
}


int check2Comp() {
    int i1 = getTwoComplementInt(0x00, 0x00);
    bool b1 = i1 == 0;
    int i2 = getTwoComplementInt(0xFF, 0xFF);
    bool b2 = i2 == -1;
    int i3 = getTwoComplementInt(0x00, 0x01);
    bool b3 = i3 == 1;
    int i4 = getTwoComplementInt(0x80, 0x01);
    bool b4 = i4 == -32767;
    //pc.printf("%d, %d, %d, %d\r\n", i1, i2, i3, i4);
    bool result = b1 && b2 && b3 && b4;
    return result;
    
}   

void Adns9500::spi_write(char addr, char data) {
    _ncs = 0;
    _spi.write(addr | 0x80);
    _spi.write(data);
    _ncs = 1;
}

int Adns9500::spi_init() {
    _spi.format(8,3);
    _spi.frequency(1000000);
    _ncs = 1;
    _ncs = 0;
    _ncs = 1;
    spi_write(0x20, 0x00);//enable laser
    return (spi_read(0x00) == 0x33); //product id
    
}
     

