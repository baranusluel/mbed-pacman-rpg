// This header has all the (extern) declarations of the globals.
// "extern" means "this is instantiated somewhere, but here's what the name
// means.
#include "globals.h"

#include "hardware.h"

// We need to actually instantiate all of the globals (i.e. declare them once
// without the extern keyword). That's what this file does!

// Hardware initialization: Instantiate all the things!
uLCD_4DGL uLCD(p9,p10,p11);             // LCD Screen (tx, rx, reset)
//SDFileSystem sd(p5, p6, p7, p8, "sd");  // SD Card(mosi, miso, sck, cs)
Serial pc(USBTX,USBRX);                 // USB Console (tx, rx)
MMA8452 acc(p28, p27, 100000);        // Accelerometer (sda, sdc, rate)
DigitalIn button1(p21);                 // Pushbuttons (pin)
DigitalIn button2(p22);
DigitalIn button3(p23);
DigitalIn button4(p24);
AnalogOut DACout(p18);                  // Speaker (pin)
PwmOut speaker(p26);
wave_player waver(&DACout);

int b1Pressed, b2Pressed, b3Pressed, b4Pressed = 0;

// Some hardware also needs to have functions called before it will set up
// properly. Do that here.
int hardware_init()
{
    // Crank up the speed
    uLCD.baudrate(3000000);
    pc.baud(115200);
        
    //Initialize pushbuttons
    button1.mode(PullUp); 
    button2.mode(PullUp);
    button3.mode(PullUp);
    button4.mode(PullUp);
    
    return ERROR_NONE;
}

GameInputs read_inputs() 
{
    GameInputs in;
    acc.readXYZGravity(&(in.ax), &(in.ay), &(in.az));
    // Debounce buttons
    if (!button1 && !b1Pressed) {
        in.b1 = true;   
    } else {
        in.b1 = false;
    }
    b1Pressed = !button1;
    if (!button2 && !b2Pressed) {
        in.b2 = true;   
    } else {
        in.b2 = false;
    }
    b2Pressed = !button2;
    if (!button3 && !b3Pressed) {
        in.b3 = true;   
    } else {
        in.b3 = false;
    }
    b3Pressed = !button3;
    if (!button4 && !b4Pressed) {
        in.b4 = true;
    } else {
        in.b4 = false;
    }
    b4Pressed = !button4;
    return in;
}
