/*
When we run this program we will read in voltage values
from pins A0(PMT) and A1(Peak Detector) from a Teensy and write
these values, along with the time in microseconds from the start 
of the program, to the serial port for retrieval by our python
program, Teensy Data.
*/

int pmt = 0;
unsigned long time;
IntervalTimer timer0;

//Function we will run in response to a 100 us timer
void SampleVoltage() {
  pmt = analogRead(0); //Voltage read from pin A0 of the Teensy
  //pd = analogRead(1); //Voltage read from pin A1 of the Teensy
  time = micros(); //Time in microseconds since start of program
  //We will write our values as an array of bytes for easy
  //transmission and retrieval
  unsigned char serialBytes[8];
  serialBytes[0] = (time >> 24) & 0xff;
  serialBytes[1] = (time >> 16) & 0xff;
  serialBytes[2] = (time >> 8) & 0xff;
  serialBytes[3] = time & 0xff;
  serialBytes[4] = (pmt >> 8) & 0xff;
  serialBytes[5] = pmt & 0xff;

  //These two bytes are here because the serial communication
  //doesn't work when only 6 bytes are sent.
  serialBytes[6] = 0x00;
  serialBytes[7] = 0x00;

//  serialBytes[0] = 0x02;
//  serialBytes[1] = 0x04;
//  serialBytes[2] = 0x08;
//  serialBytes[3] = 0x10;
//  serialBytes[4] = 0x20;
//  serialBytes[5] = 0x40;
//  serialBytes[6] = 0x00;
//  serialBytes[7] = 0x00;
  
  Serial.write(serialBytes,8);

  //pmt += 10;
}

void timer_setup() {
  timer0.begin(SampleVoltage, 110);  //SampleVoltage will run every 100 us
}
  
void timer_stop() {
  timer0.end();
}


void setup()
{
  Serial.begin(9600);
  delay(2000); //To allow time for serial port to begin
  timer_setup(); //Run timer
}

void loop()
{

}

