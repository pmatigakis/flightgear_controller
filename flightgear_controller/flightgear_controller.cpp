#include <math.h>

#include "mbed.h"
#include "MMA8451Q.h"

#define MAX_PITCH 60.0
#define MAX_ROLL 60.0
#define BAUD_RATE 57600

Serial serial(USBTX, USBRX);
MMA8451Q acc(PTE25, PTE24, 0x1d<<1);
DigitalOut led(LED1);
Ticker ticker;

void update_controls(){
    float x, y, z;
    float elevator, aileron, rudder;
    float roll, pitch;
    
    //Set rudder to 0 beacause there is no way to control this using the board
    rudder = 0.0;
    
    //Get the accelerations
    x = acc.getAccX();
    y = acc.getAccY();
    z = acc.getAccZ();
        
    //Calculate the roll and pitch of the board
    roll = (atan2(-y, z) * 180.0) / 3.1415;
    pitch = (atan2(x, sqrt(y * y + z * z)) * 180.0) / 3.1415;
    
    
    //Convert roll and pitch to the elevator and aileron states that will be transmited to flightgear
    //These values should be between -1.0 and 1.0
    if (roll > MAX_ROLL)
        roll = MAX_ROLL;
    else if(roll < -MAX_ROLL)
        roll = -MAX_ROLL;
        
    if (pitch > MAX_PITCH)
        pitch = MAX_PITCH;
    else if(pitch < -MAX_PITCH)
        pitch = -MAX_PITCH;
        
    elevator = pitch / MAX_PITCH;
    aileron = roll / MAX_ROLL;
        
    //Transmit the aileron, elevator and rudder states to flightgear
    serial.printf("%f\t%f\t%f\r\n", elevator, aileron, rudder);    
}

int main(){
    serial.baud(BAUD_RATE);
    
    //Transmit commands to flightgear every 50ms
    ticker.attach(&update_controls, 0.05);
    
    while(1){
        led = !led;
        wait(0.5);
    }
}