/*
When we run this program we will read in voltage values
from pins A0(PMT) and A1(Peak Detector) from a Teensy and write
these values, along with the time in microseconds from the start 
of the program, to the serial port for retrieval by our python
program, Teensy Data.
*/

int pmt;
int pd;
unsigned long time;
IntervalTimer timer0;

//Function we will run in response to a 100 us timer
void SampleVoltage() {
  pmt = analogRead(0); //Voltage read from pin A0 of the Teensy
  //pd = analogRead(1); //Voltage read from pin A1 of the Teensy
  time = micros(); //Time in microseconds since start of program
  //We will write our values as an array of bytes for easy
  //transmission and retrieval
  unsigned char serialBytes[6];
  serialBytes[0] = (time >> 24) & 0xff;
  serialBytes[1] = (time >> 16) & 0xff;
  serialBytes[2] = (time >> 8) & 0xff;
  serialBytes[3] = time & 0xff;
  serialBytes[4] = (pmt >> 8) & 0xff;
  serialBytes[5] = pmt & 0xff;

//  serialBytes[0] = 0;
//  serialBytes[1] = 0;
//  serialBytes[2] = 0;
//  serialBytes[3] = 0;
//  serialBytes[4] = 0;
//  serialBytes[5] = 0;
//  serialBytes[6] = 0;
//  serialBytes[7] = 0;
  
  Serial.write(serialBytes,6);
}

void timer_setup() {
  timer0.begin(SampleVoltage, 100);  //SampleVoltage will run every 100 us
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

