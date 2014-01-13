/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "MMA8451Q.h"

#define INT_SOURCE        0x0C 
#define REG_WHO_AM_I      0x0D
#define HP_FILTER_CUTOFF  0x0F 
#define PULSE_CFG         0x21 
#define PULSE_SRC         0x22 
#define PULSE_THSX        0x23 
#define PULSE_THSY        0x24 
#define PULSE_THSZ        0x25 
#define PULSE_TMLT        0x26 
#define PULSE_LTCY        0x27 
#define PULSE_WIND        0x28 
#define REG_CTRL_REG_1    0x2A 
#define CTRL_REG2         0x2B
#define CTRL_REG4         0x2D 
#define CTRL_REG5         0x2E 
#define REG_OUT_X_MSB     0x01
#define REG_OUT_Y_MSB     0x03
#define REG_OUT_Z_MSB     0x05

#define UINT14_MAX        16383

MMA8451Q::MMA8451Q(PinName sda, PinName scl, int addr) : m_i2c(sda, scl), m_addr(addr) {
    // activate the peripheral
    uint8_t data[2] = {REG_CTRL_REG_1, 0x01};
    writeRegs(data, 2);
}

MMA8451Q::~MMA8451Q() { }

uint8_t MMA8451Q::getWhoAmI() {
    uint8_t who_am_i = 0;
    readRegs(REG_WHO_AM_I, &who_am_i, 1);
    return who_am_i;
}

float MMA8451Q::getAccX() {
//divide by 4096 b/c MMA output is 4096 counts per g so this f outputs accelorometer value formatted to g (gravity)
    return (float(getAccAxis(REG_OUT_X_MSB))/4096.0);
}

float MMA8451Q::getAccY() {
    return (float(getAccAxis(REG_OUT_Y_MSB))/4096.0);
}

float MMA8451Q::getAccZ() {
    return (float(getAccAxis(REG_OUT_Z_MSB))/4096.0);
}

void MMA8451Q::getAccAllAxis(float * res) {
    res[0] = getAccX();
    res[1] = getAccY();
    res[2] = getAccZ();
}

int16_t MMA8451Q::getAccAxis(uint8_t addr) {
    int16_t acc;
    uint8_t res[2];
    readRegs(addr, res, 2);

    acc = (res[0] << 6) | (res[1] >> 2);
    if (acc > UINT14_MAX/2)
        acc -= UINT14_MAX;

    return acc;
}

void MMA8451Q::setDoubleTap(void){
//Implemented directly from Freescale's AN4072 
//Added to MMA8451Q lib

    uint8_t CTRL_REG1_Data;
//    int adds;
   uint8_t data[2] = {REG_CTRL_REG_1, 0x08};
    
    //400 Hz, Standby Mode
    writeRegs(data,2);
    
    //Enable X, Y and Z Double Pulse with DPA = 0 no double pulse abort    
    data[0]=PULSE_CFG;data[1]=0x2A;
    writeRegs(data,2);
    
    //SetThreshold 3g on X and Y and 5g on Z
    //Note: Every step is 0.063g
    //3 g/0.063g = 48 counts
    //5g/0.063g = 79 counts
    data[0]=PULSE_THSX;data[1]=0x30;
    writeRegs(data,2);//Set X Threshold to 3g 
    data[0]=PULSE_THSY;data[1]=0x30;
    writeRegs(data,2);//Set Y Threshold to 3g 
    data[0]=PULSE_THSZ;data[1]=0x4F;
    writeRegs(data,2);//Set Z Threshold to 5g

    //Set Time Limit for Tap Detection to 60 ms LP Mode
    //Note: 400 Hz ODR, Time step is 1.25 ms per step
    //60 ms/1.25 ms = 48 counts 
    data[0]=PULSE_TMLT;data[1]=0x30;
    writeRegs(data,2);//60 ms
    
    //Set Latency Time to 200 ms
    //Note: 400 Hz ODR LPMode, Time step is 2.5 ms per step 00 ms/2.5 ms = 80 counts
    data[0]=PULSE_LTCY;data[1]=0x50;
    writeRegs(data,2);//200 ms
    
    //Set Time Window for second tap to 300 ms
    //Note: 400 Hz ODR LP Mode, Time step is 2.5 ms per step
    //300 ms/2.5 ms = 120 counts
    data[0]=PULSE_WIND;data[1]=0x78;
    writeRegs(data,2);//300 ms
    
    //Route INT1 to System Interrupt
    data[0]=CTRL_REG4;data[1]=0x08;
    writeRegs(data,2);//Enable Pulse Interrupt in System CTRL_REG4
    data[0]=CTRL_REG5;data[1]=0x08; 
    writeRegs(data,2);//Route Pulse Interrupt to INT1 hardware Pin CTRL_REG5

    //Set the device to Active Mode
    readRegs(0x2A,&CTRL_REG1_Data,1);//Read out the contents of the register 
    CTRL_REG1_Data |= 0x01; //Change the value in the register to Active Mode.
    data[0]=REG_CTRL_REG_1; 
    data[1]=CTRL_REG1_Data;
    writeRegs(data,2);//Write in the updated value to put the device in Active Mode
}


void MMA8451Q::readRegs(int addr, uint8_t * data, int len) {
    char t[1] = {addr};
    m_i2c.write(m_addr, t, 1, true);
    m_i2c.read(m_addr, (char *)data, len);
}



void MMA8451Q::writeRegs(uint8_t * data, int len) {
    m_i2c.write(m_addr, (char *)data, len);
}
