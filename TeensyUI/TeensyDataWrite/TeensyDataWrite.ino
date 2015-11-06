/*
When we run this program we will read in voltage values
from pins A0(PMT) and A1(Peak Detector) from a Teensy and write
these values, along with the time in microseconds from the start 
of the program, to the serial port for retrieval by our python
program, Teensy Data.
*/

//Duty cycle for each pin - Change accordingly - Max of 0.5 increments 
#define D_CYCLE1 100
#define D_CYCLE2 100
#define D_CYCLE3 100
#define D_CYCLE4 100

//new board
int pwmPin1 = 5;//Waste Well
int pwmPin2 = 6;//Source Well
int pwmPin3 = 4;//Separation Well
int pwmPin4 = 3;//HVcontrol


//Variables
int RESOLUTION = 4096; //12 bits max resolution (DO NOT CHANGE)

int incomingByte=0;

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
    //Setting the pins to 1kHz frequency
  analogWriteFrequency(pwmPin1, 1000);
  analogWriteFrequency(pwmPin2, 1000);
  analogWriteFrequency(pwmPin3, 1000);
  analogWriteFrequency(pwmPin4, 1000);
  //Set the pins as outputs
  pinMode(pwmPin1, OUTPUT);
  pinMode(pwmPin2, OUTPUT);  
  pinMode(pwmPin3, OUTPUT);
  pinMode(pwmPin4, OUTPUT);
  
  analogWriteResolution(12); // Set up DAC resolution
  analogReadResolution(12);
  analogWrite(A14, RESOLUTION/2); // Will output 1.65 volts on DAC to illuminate
                                  // the laser at the same intensity as modulated laser
                                  
  Serial.begin(9600);
  delay(2000); //To allow time for serial port to begin
  timer_setup(); //Run timer
}

void loop()
{
  //----------------------------------------------------------------------------------------
// ---------------------------------FOR HIGH VOLTAGE----------------------------------------
//-----------------------------------------------------------------------------------------
  while(Serial.available())
  {    
    incomingByte = Serial.read();
    
    // Change values below to modify the voltage at each of the pins)
    switch (incomingByte)//Cases are in DEC format 
    {
      case 49://Number 1 in 8-bit decimal
        // HV on
        analogWrite(pwmPin1, RESOLUTION*D_CYCLE1/100);
        analogWrite(pwmPin2, RESOLUTION*D_CYCLE2/100);
        analogWrite(pwmPin3, RESOLUTION*D_CYCLE3/100);        
        analogWrite(pwmPin4, RESOLUTION*D_CYCLE4/100);//HV on
        break;
      case 50://Number 2 in 8-bit decimal
        // Separation
        analogWrite(pwmPin1, RESOLUTION*(D_CYCLE1-83)/100);
        analogWrite(pwmPin2, RESOLUTION*(D_CYCLE2-83)/100);
        analogWrite(pwmPin3, RESOLUTION*(D_CYCLE3-94)/100);
        analogWrite(pwmPin4, RESOLUTION*D_CYCLE4/100);
        break;
      case 51:
        // Injection
        analogWrite(pwmPin1, RESOLUTION*(D_CYCLE1-100)/100);
        analogWrite(pwmPin2, RESOLUTION*(D_CYCLE2-78)/100);
        analogWrite(pwmPin3, RESOLUTION*(D_CYCLE3-78)/100);
        analogWrite(pwmPin4, RESOLUTION*D_CYCLE4/100);
        break;
      default: 
        // HV off
        analogWrite(pwmPin1, RESOLUTION*D_CYCLE1/100);
        analogWrite(pwmPin2, RESOLUTION*D_CYCLE2/100);
        analogWrite(pwmPin3, RESOLUTION*D_CYCLE3/100);        
        analogWrite(pwmPin4, 0);//HV off
        break;
    }
  }
}

