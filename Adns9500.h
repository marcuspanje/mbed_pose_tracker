#ifndef ADNS9500_H
#define ADNS9500_H

#include "mbed.h"
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

class Adns9500 {
    public:
        Adns9500(PinName mosi, PinName miso, PinName sclk, PinName ncs);
        char spi_read(int addr);
        void spi_read_xy(int &x, int &y);
        void spi_write(char addr, char data);  
        int spi_init();

    private:
    //DigitalOut ncs(p16);
        SPI _spi;
        DigitalOut _ncs;
};

#endif
