#include "mbed.h"
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include "Adns9500.h"
#include "LSM9DS1.h"

using namespace std;


DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
//InterruptIn transmitting(p8);
DigitalOut transmit_pin(p15);


LSM9DS1 imu(p9, p10, 0xD6, 0x3C);
Adns9500 mouse(p5,p6,p7,p16);//mosi,miso,sclk,ncs

Ticker transmitter;

Serial pc(USBTX, USBRX);

const double transmit_freq = 10.0;
double pose[6]; // x,y,z,wx,wy,wz
const int bufferSz = 100;
char buffer[bufferSz];

//mouse calibration params
double count_per_inch = 1400.0;
double cmeter_per_count = 2.54/count_per_inch;


void transmit_serial() {
    if (!transmit_pin) {
        transmit_pin = 1;
    }
}

void transmit_interrupt() {
    led1 = 1;
    int i = 0;
    while(buffer[i] != '\0') {
        //poll until buffer is writeable
        while(!pc.writeable());
        pc.putc(buffer[i]);
        i++;
    }
    led1 = 0;
    transmit_pin = 0;
}

      
     
int main() {

    for (int i = 0; i < 6; i++) {
         pose[i] = 0.0;
    }
    if (!mouse.spi_init()) {
        led1 = 1;
        return 0;
    }
    imu.begin();
    if (!imu.begin()) {
        pc.printf("error: failed to communicate with LSM9DS1.\n");
    }
    imu.calibrate();
    float gyro_dt = 0.0;
    int serialSendPeriod_ms = 1000*(1.0/transmit_freq);
    int x;
    int y;
    
    Timer gyro_timer;
    gyro_timer.start();
    Timer serial_timer;
    serial_timer.start();
    while(1) {

        led2 = !led2;
        mouse.spi_read_xy(x, y);
        pose[0] = pose[0] + cmeter_per_count*x;
        pose[1] = pose[1] - cmeter_per_count*y;

        imu.readTemp();
        imu.readGyro();
        gyro_dt = 0.000001*gyro_timer.read_us();
        gyro_timer.reset();
        
        pose[3] = pose[3] + gyro_dt * imu.calcGyro(imu.gx);
        pose[4] = pose[4] - gyro_dt * imu.calcGyro(imu.gy);
        pose[5] = pose[5] + gyro_dt * imu.calcGyro(imu.gz);
            
        if (serial_timer.read_ms() >= serialSendPeriod_ms) {
            serial_timer.reset();    
            pc.printf("%.4f %.4f %.4f %.4f %.4f %.4f\r\n", 
                pose[0], pose[1], pose[2], pose[3], pose[4], pose[5]);
        }
            
   } 



/*
    for (int i = 0; i < 6; i++) {
         pose[i] = 0.0;
    }

    if (!mouse.spi_init()) {
        led1 = 1;
        return 0;
    }
    transmitter.attach(&transmit_serial, 1.0/transmit_freq);
    transmitting.rise(&transmit_interrupt);
    char motion;
    int x;
    int y;
    while(1) {
        led2 = !led2;
        mouse.spi_read_xy(x, y);
        pose[0] = pose[0] + cmeter_per_count*x;
        pose[1] = pose[1] - cmeter_per_count*y;
        transmitting.disable_irq();
        snprintf(buffer, bufferSz, "%.5f %.5f %.5f %.5f %.5f %.5f\r\n", 
    pose[0], pose[1], pose[2], pose[3], pose[4], pose[5]);
        transmitting.enable_irq();
        wait_us(100);
    }
*/

}

