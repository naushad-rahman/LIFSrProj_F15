//Duty cycle for each pin - Change accordingly - Max of 0.5 increments 
#define D_CYCLE1 100
#define D_CYCLE2 100
#define D_CYCLE3 100
#define D_CYCLE4 100

//new board
int pwmPin1 = 5;
int pwmPin2 = 6;
int pwmPin3 = 4;
int pwmPin4 = 3;//HVcontrol


//Variables
int RESOLUTION = 4096; //12 bits max resolution (DO NOT CHANGE)

int incomingByte=0;

#define BAUD_RATE 115200
#define TIMER_INT_MICROS 200
#define LENGTH_OF_DAC 10
#define N_FILTER_LENGTH 37
#define LENGTH_OF_SIGNAL 37

float in_array[LENGTH_OF_SIGNAL]; 
int after_LPF;

int n_samp; 
int time;
bool volatile LP_Flag = false;
float correction_factor; 

IntervalTimer timer0;
int dac_lut[LENGTH_OF_DAC]; 
float twopi = 3.14159265359 * 2;
int j;


// Coefficients for FIR low-pass filter
// Order: 31
// Fs = 1000
// Fpass = 5
// Fstop = 30
// Density Factor 20
float lp_filter_coeff[N_FILTER_LENGTH] = {
           0.000326, 0.000692, 0.001207, 0.001891, 0.002756, 
           0.003806, 0.005036, 0.006430, 0.007961, 0.009591, 
           0.011275, 0.012958, 0.014582, 0.016086, 0.017413, 
           0.018507, 0.019325, 0.019829, 0.020000, 0.019829, 
           0.019325, 0.018507, 0.017413, 0.016086, 0.014582, 
           0.012958, 0.011275, 0.009591, 0.007961, 0.006430, 
           0.005036, 0.003806, 0.002756, 0.001891, 0.001207, 0.000692, 0.000326};
           
//----------------------------------------------------------------------

void LUT(){
   for(int i = 0; i < LENGTH_OF_DAC; i++){
     dac_lut[i] = (int) ((cos(twopi*((float) i)/ LENGTH_OF_DAC) + 1)*2050);   
   }
}
//------------------------------------------------------------
void ISR(){
  // Set up the DAC
  //if(j >= LENGTH_OF_DAC){
  //  j = 0;
  //}
  //analogWrite(A14, dac_lut[j]); 
  //j++;
  
  in_array[n_samp] = analogRead(0);
  
  n_samp++;
  // Set up ADC and Filter
  if(n_samp >= LENGTH_OF_SIGNAL){
    n_samp = 0; 
    LP_Flag = true; 
  }
  
}
//-------------------------------------------------------

float execute_LPF(){
  int lp_fifo_num = 36; 
  
  float y_LPF = in_array[lp_fifo_num]*lp_filter_coeff[0];
  
  for(int fir_counter = 1; fir_counter < N_FILTER_LENGTH; fir_counter++){
        int fir_index = lp_fifo_num + fir_counter;
      if (fir_index >= N_FILTER_LENGTH) {
        fir_index -= N_FILTER_LENGTH;
      }
      y_LPF += in_array[fir_index] * lp_filter_coeff[fir_counter];
  
    }  
   //Serial.print("\n\r y_LPF: ");
   // Serial.print(y_LPF);
    
    return y_LPF/correction_factor; 
}


//-----------------------------------------------------------------

void timer_setup() { //setup interrupts
  timer0.begin(ISR, TIMER_INT_MICROS);  
} //timer_setup() 

//---------------------------------------------------------------------

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
  analogWrite(A14, RESOLUTION/2); // Will output 1.65 volts on DAC to illuminate
                                  // the laser at the same intensity as modulated laser
  //j = 0;
  correction_factor = 0; 
  
  for(int i = 0; i < N_FILTER_LENGTH; i++){
    correction_factor += lp_filter_coeff[i]; 
  } 
  
  Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(5000);
  //LUT();
  Serial.print("\n\rHello! Right now we are low pass filtering the input of DC PMT signal."); 
  Serial.print("\n\r Correction factor: ");
  Serial.print(correction_factor);
  delay(1000); 
  timer_setup();
}

void loop()
{  
  if(LP_Flag){
      
      after_LPF = (int) 10*execute_LPF();
      //Serial.print("\n\r after_LPF: ");
      //Serial.print(after_LPF);
      
      time = micros();
    
      unsigned char serialBytes[8]; 
      serialBytes[0] = (time >> 24) & 0xff;
      serialBytes[1] = (time >> 16) & 0xff;
      serialBytes[2] = (time >> 8) & 0xff; 
      serialBytes[3] = time & 0xff; 
      serialBytes[4] = (after_LPF >> 24) & 0xff;
      serialBytes[5] = (after_LPF >> 16) & 0xff;
      serialBytes[6] = (after_LPF >> 8) & 0xff; 
      serialBytes[7] = after_LPF & 0xff; 
      Serial.println(after_LPF);
      //Serial.write(serialBytes,8);
 
  }
  
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
        analogWrite(pwmPin3, RESOLUTION*(D_CYCLE3-94)/100);
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
