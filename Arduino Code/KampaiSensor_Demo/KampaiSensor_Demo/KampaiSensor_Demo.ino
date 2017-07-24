/*****************************************************************
XBee_Serial_Passthrough.ino

Set up a software serial port to pass data between an XBee Shield
and the serial monitor.

Hardware Hookup:
  The XBee Shield makes all of the connections you'll need
  between Arduino and XBee. If you have the shield make
  sure the SWITCH IS IN THE "DLINE" POSITION. That will connect
  the XBee's DOUT and DIN pins to Arduino pins 2 and 3.

*****************************************************************/
// We'll use SoftwareSerial to communicate with the XBee:
#include <SoftwareSerial.h>
  // XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
  // XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX

#define SensorSamplePeriod  500 //in ms

#define SCL_PIN 5  //A5       //Note that if you are using the I2C based sensors, you will need to download and
#define SCL_PORT PORTC    //install the "SoftI2CMaster" as "Wire" does not support repeated start...
#define SDA_PIN 4  //A4         //References:
#define SDA_PORT PORTC    //  http://playground.arduino.cc/Main/SoftwareI2CLibrary

#include <SoftI2CMaster.h>  //  https://github.com/felias-fogg/SoftI2CMaster
#define I2C_TIMEOUT 1000  // Sets Clock Stretching up to 1sec
#define I2C_FASTMODE 1    // Sets 400kHz operating speed

//I2C globals (using SoftI2CMaster libary) - MEMs Kionix Sensor 
int I2C_check = 0;
unsigned int lineClear = 0;
unsigned int j =0;

//Sensor Global Variables
unsigned int BH1747_DeviceAddress = 0x70;  //this is the 8bit address, 7bit address = 0x4E
unsigned int BH1747_RED_LB = 0;
unsigned int BH1747_RED_HB = 0;
unsigned int BH1747_GRN_LB = 0;
unsigned int BH1747_GRN_HB = 0;
unsigned int BH1747_BLU_LB = 0;
unsigned int BH1747_BLU_HB = 0;
unsigned int BH1747_CLR_LB = 0;
unsigned int BH1747_CLR_HB = 0;
unsigned int BH1747_IR_LB = 0;
unsigned int BH1747_IR_HB = 0;
unsigned int BH1747_GRN2_LB = 0;
unsigned int BH1747_GRN2_HB = 0;

unsigned int BH1747_RED_OUT = 0;
unsigned int BH1747_GRN_OUT = 0;
unsigned int BH1747_BLU_OUT = 0;
unsigned int BH1747_CLR_OUT = 0;
unsigned int BH1747_IR_OUT = 0;
unsigned int BH1747_GRN2_OUT = 0;

void setup()
{
  // Set up both ports at 9600 baud. This value is most important
  // for the XBee. Make sure the baud rate matches the config
  // setting of your XBee.
  XBee.begin(9600);
  //Serial.begin(9600);

  I2C_check = i2c_init();
  if(I2C_check == false){
     while(1){
       XBee.write("I2C Init Failed (SDA or SCL may not be pulled up!");
       XBee.write(0x0A); //Print Line Feed
       XBee.write(0x0D); //Print Carrage Return
       delay(500);
     }
  }

  //BH1747 Init Sequence
  // 1. Mode Control 1 (0x41), write (0x7B) //Highest Gain for IR and Color, Longest Measurement Mode
  // 2. Mode Control 2 (0x42), write (0x90) //VALID1, EN1...
  
  i2c_start(BH1747_DeviceAddress);  //This needs the 8 bit address (7bit Device Address + RW bit... Read = 1, Write = 0)
  i2c_write(0x41);
  i2c_write(0x7B);
  i2c_stop();
  
  i2c_start(BH1747_DeviceAddress);  //This needs the 8 bit address (7bit Device Address + RW bit... Read = 1, Write = 0)
  i2c_write(0x42);
  i2c_write(0x90);
  i2c_stop();
}

void loop()
{

  //Clear the Display to make data easier to read
  for(j = 0; j<lineClear; j++){
    XBee.write("\033[F");
    XBee.write("\033[J");
  }
  lineClear = 0;

  // Start Getting Data from COLOR Sensor
  i2c_start(BH1747_DeviceAddress);
  i2c_write(0x50);
  i2c_rep_start(BH1747_DeviceAddress | 1);  // Or-ed with "1" for read bit
  BH1747_RED_LB = i2c_read(false);
  BH1747_RED_HB = i2c_read(false);
  BH1747_GRN_LB = i2c_read(false);
  BH1747_GRN_HB = i2c_read(false);
  BH1747_BLU_LB = i2c_read(false);
  BH1747_BLU_HB = i2c_read(false);  
  
  BH1747_CLR_LB = i2c_read(false);
  BH1747_CLR_HB = i2c_read(false);  
  BH1747_IR_LB = i2c_read(false);
  BH1747_IR_HB = i2c_read(false);
  BH1747_GRN2_LB = i2c_read(false);
  BH1747_GRN2_HB = i2c_read(true);
  i2c_stop();

  BH1747_RED_OUT = (BH1747_RED_HB<<8) | (BH1747_RED_LB);
  BH1747_GRN_OUT = (BH1747_GRN_HB<<8) | (BH1747_GRN_LB);
  BH1747_BLU_OUT = (BH1747_BLU_HB<<8) | (BH1747_BLU_LB);
  BH1747_CLR_OUT = (BH1747_CLR_HB<<8) | (BH1747_CLR_LB);
  BH1747_IR_OUT = (BH1747_IR_HB<<8) | (BH1747_IR_LB);
  BH1747_GRN2_OUT = (BH1747_GRN2_HB<<8) | (BH1747_GRN2_LB);
  
  XBee.write("BH1748 (Red)   = ");
  XBee.print(BH1747_RED_OUT);
  XBee.write(" ADC Counts");
  XBee.write(0x0A);  //Print Line Feed
  XBee.write(0x0D);  //Print Carrage Return
  XBee.write("BH1748 (Green) = ");
  XBee.print(BH1747_GRN_OUT);
  XBee.write(" ADC Counts");
  XBee.write(0x0A);  //Print Line Feed
  XBee.write(0x0D);  //Print Carrage Return
  XBee.write("BH1748 (Blue)  = ");
  XBee.print(BH1747_BLU_OUT);
  XBee.write(" ADC Counts");
  XBee.write(0x0A);  //Print Line Feed
  XBee.write(0x0D);  //Print Carrage Return

  /*
  XBee.write("BH1748 (Clear) = ");
  XBee.print(BH1747_CLR_OUT);
  XBee.write(" ADC Counts");
  XBee.write(0x0A);  //Print Line Feed
  XBee.write(0x0D);  //Print Carrage Return
  XBee.write("BH1748 (IR) = ");
  XBee.print(BH1747_IR_OUT);
  XBee.write(" ADC Counts");
  XBee.write(0x0A);  //Print Line Feed
  XBee.write(0x0D);  //Print Carrage Return
  XBee.write("BH1748 (Green 2) = ");
  XBee.print(BH1747_GRN2_OUT);
  XBee.write(" ADC Counts");
  XBee.write(0x0A);  //Print Line Feed
  XBee.write(0x0D);  //Print Carrage Return
  */
  
  lineClear += 6;
  /*
  if (Serial.available())
  { // If data comes in from serial monitor, send it out to XBee
    XBee.write(Serial.read());
  }
  */
  delay(SensorSamplePeriod);  //Add some Loop Delay

  /*
  if (XBee.available())
  { // If data comes in from XBee, send it out to serial monitor
    Serial.write(XBee.read());
  }
  */
}
