/*
To use the Serial Port (USB), we must not use pin 0 and 1 (Tx and Rx) for digital 
input or output. I believe this is always true for arduino but maybe for teensy too.
*/
//Pin numbers - Change accordingly
int pwmPin1 = 4;
int pwmPin2 = 5;
int pwmPin3 = 20;
int pwmPin4 = 2;//HV control
//Variables
int RESOLUTION = 256;//8 bits max resolution (DO NOT CHANGE)
//Duty cycle for each pin - Change accordingly - Max of 0.5 increments
float d_cycle1 = 100;
float d_cycle2 = 100;
float d_cycle3 = 100;
float d_cycle4 = 50; //HV on: straight 3.3V DC
int incomingByte=0;
//Analog Setup
float phase = 0.0;
float twopi = 3.14159 * 2;
elapsedMicros usec = 0;

void setup()
{
  //Setup the USB serial connection. Teensy always talks at this speed (DO NOT CHANGE)
  Serial.begin(9600);
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
  //Analog output
  analogWriteResolution(12);
}

void loop()
{  
  while(Serial.available())
  {
    incomingByte = Serial.read();
    
    switch (incomingByte)//Cases are in DEC format 
    {
      case 49://Number 1 in 8-bit decimal
        // HV ON only
        analogWrite(pwmPin1, RESOLUTION*d_cycle1/100);
        analogWrite(pwmPin2, RESOLUTION*d_cycle2/100);
        analogWrite(pwmPin3, RESOLUTION*d_cycle3/100);
        analogWrite(pwmPin4, RESOLUTION*d_cycle4/100);
        break;
      case 50://Number 2 in 8-bit decimal
        // All pins at max output, HV ON
        analogWrite(pwmPin1, RESOLUTION*(d_cycle1-100)/100);
        analogWrite(pwmPin2, RESOLUTION*(d_cycle2-100)/100);
        analogWrite(pwmPin3, RESOLUTION*(d_cycle3-100)/100);
        analogWrite(pwmPin4, RESOLUTION*d_cycle4/100);
        break;
      case 51:
        // statements
        analogWrite(pwmPin1, RESOLUTION*(d_cycle1-70)/100);
        analogWrite(pwmPin2, RESOLUTION*(d_cycle2-100)/100);
        analogWrite(pwmPin3, RESOLUTION*(d_cycle3-70)/100);
        analogWrite(pwmPin4, RESOLUTION*d_cycle4/100);
        break;
      default: 
        // Everything OFF
        analogWrite(pwmPin1, RESOLUTION*d_cycle1/100);
        analogWrite(pwmPin2, RESOLUTION*d_cycle2/100);
        analogWrite(pwmPin3, RESOLUTION*d_cycle3/100);
        analogWrite(pwmPin4, 0);
        break;
    }
  }
  //analog output
//  float val = sin(phase) * 2000.0 + 2050.0;
//  analogWrite(A14, (int)val);
//  phase = phase + 0.02;
//  if (phase >= twopi) phase = 0;
//  while (usec < 50) ; // wait
//  usec = usec - 50;
}
