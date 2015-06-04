#define D_CYCLE1 100
#define D_CYCLE2 100
#define D_CYCLE3 100
#define D_CYCLE4 100


/*
To use the Serial Port (USB), we must not use pin 0 and 1 (Tx and Rx) for digital 
input or output. I believe this is always true for arduino but maybe for teensy too
*/
//Pin numbers - Change accordingly
//int pwmPin1 = 4;
//int pwmPin2 = 5;
//int pwmPin3 = 20;
//int pwmPin4 = 6;//HV control

//new board
int pwmPin1 = 6;
int pwmPin2 = 5;
int pwmPin3 = 4;
int pwmPin4 = 3;//HVcontrol


//Variables
int RESOLUTION = 256;//8 bits max resolution (DO NOT CHANGE)
//Duty cycle for each pin - Change accordingly - Max of 0.5 increments
//float d_cycle1 = 100;
//float d_cycle2 = 100;
//float d_cycle3 = 100;
////float d_cycle4 = 33.5;
//
//float d_cycle4 = 100; // for new board

/*The first test that we ran, all 3 transistors were active and we found that at 33.5% modulation gave about 1.1k*/
int incomingByte=0;

void setup()
{
  //Setup the USB serial connection. Teensy always talks at this speed
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
}

void loop()
{  
  while(Serial.available())
  {
    incomingByte = Serial.read();
    
    switch (incomingByte)//Cases are in DEC format 
    {
      case 49://Number 1 in 8-bit decimal
        // statements
        analogWrite(pwmPin1, RESOLUTION*D_CYCLE1/100);
        analogWrite(pwmPin2, RESOLUTION*D_CYCLE2/100);
        analogWrite(pwmPin3, RESOLUTION*D_CYCLE3/100);        
        analogWrite(pwmPin4, RESOLUTION*D_CYCLE4/100);//HV on
        break;
      case 50://Number 2 in 8-bit decimal
        // Flowing/Testing
        analogWrite(pwmPin1, RESOLUTION*(D_CYCLE1-83)/100);
        analogWrite(pwmPin2, RESOLUTION*(D_CYCLE2-83)/100);
        analogWrite(pwmPin3, RESOLUTION*(D_CYCLE3-100)/100);
        analogWrite(pwmPin4, RESOLUTION*D_CYCLE4/100);
        break;
      case 51:
        //Loading
        analogWrite(pwmPin1, RESOLUTION*(D_CYCLE1-100)/100);
        analogWrite(pwmPin2, RESOLUTION*(D_CYCLE2-78)/100);
        analogWrite(pwmPin3, RESOLUTION*(D_CYCLE3-78)/100);
        analogWrite(pwmPin4, RESOLUTION*D_CYCLE4/100);
        break;
      default: 
        // statements
        analogWrite(pwmPin1, RESOLUTION*D_CYCLE1/100);
        analogWrite(pwmPin2, RESOLUTION*D_CYCLE2/100);
        analogWrite(pwmPin3, RESOLUTION*D_CYCLE3/100);        
        analogWrite(pwmPin4, 0);//HV off
        break;
    }
  }
}
