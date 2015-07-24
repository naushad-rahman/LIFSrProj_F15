/*
Low Pass filtering PMT DC signal.

*/
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

int in_array[LENGTH_OF_SIGNAL]; 
float after_LPF;

int n_samp; 
int time;
bool volatile LP_Flag = false;
float correction_factor; 

IntervalTimer timer0;
int dac_lut[LENGTH_OF_DAC]; 
float twopi = 3.14159265359 * 2;
int j;

// 1=raised cosine (calculated), 2=impulse, 3=raised cosine from lut, 4=rectangular pulse, 
// 5=rectangular modulation of raised cosine, 6=Gaussian modulation of raised cosine

#define SIMULATED_SIGNAL_SELECTION 1
#define LENGTH_OF_TEST_SIGNAL 1000
float test_signal[LENGTH_OF_TEST_SIGNAL];
int volatile index_test_signal;

//-----------------------------------------------------------------
void setup_test_signal() {
  int mid, width;
  switch (SIMULATED_SIGNAL_SELECTION) {
    case 1: // raised cosine as float in range [0,2]
      for (int ii = 0; ii < LENGTH_OF_TEST_SIGNAL; ii++) {
        test_signal[ii] = 1.0 + cos(twopi*((float) ii)/ LENGTH_OF_DAC);
      }
      break;
    case 2: // impulse with range [0,1]
      for (int ii=0; ii<LENGTH_OF_TEST_SIGNAL; ii++) test_signal[ii] = 0.0;
      test_signal[LENGTH_OF_TEST_SIGNAL/2] = 1.0;
      break;
    case 3: // raised cosine using look up table, range is [0,4094]
      for (int ii=0; ii<LENGTH_OF_TEST_SIGNAL; ii++) {
        test_signal[ii] = dac_lut[ii % LENGTH_OF_DAC];
      }
      break;
    case 4: // rectangular pulse with range [0,1]
      mid = LENGTH_OF_TEST_SIGNAL/2; width = LENGTH_OF_TEST_SIGNAL/4;
      for (int ii=0; ii<LENGTH_OF_TEST_SIGNAL; ii++) test_signal[ii] = 0.0;
      for (int ii=mid-width/2; ii<mid+width/2; ii++) test_signal[ii] = 1.0;
      break;
    case 5: // rectangular modulation of raised cosine, range [0,2]
      mid = LENGTH_OF_TEST_SIGNAL/2; width = LENGTH_OF_TEST_SIGNAL/4;
      for (int ii=0; ii<LENGTH_OF_TEST_SIGNAL; ii++) test_signal[ii] = 0.0;
      for (int ii=mid-width/2; ii<mid+width/2; ii++) test_signal[ii] = 1.0;
      for (int ii = 0; ii < LENGTH_OF_TEST_SIGNAL; ii++) {
        test_signal[ii] *= 1.0 + cos(twopi*((float) ii)/ LENGTH_OF_DAC);
      }
      break;
    case 6: // Gaussian modulation of raised cosine, range [0,2]
      mid = LENGTH_OF_TEST_SIGNAL/2; width = LENGTH_OF_TEST_SIGNAL/8;
      for (int ii = 0; ii < LENGTH_OF_TEST_SIGNAL; ii++) {
        test_signal[ii] = exp( -pow((ii-mid),2.0)/(2.0*pow(width,2.0)) );
        test_signal[ii] *= (1.0 + cos(twopi*((float) ii)/ LENGTH_OF_DAC));
      }
      break;
    default: 
      break;
   }  
}

//-----------------------------------------------------------------

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
  
  in_array[n_samp] = test_signal[index_test_signal];//analogRead(0);
  n_samp++;
  if(n_samp >= LENGTH_OF_SIGNAL){
    n_samp = 0; 
    LP_Flag = true; 
  }
  index_test_signal++;
  if(index_test_signal >= LENGTH_OF_TEST_SIGNAL) index_test_signal = 0;
}
//-------------------------------------------------------

float execute_LPF(){
  int lp_fifo_num = 36; 
  
  float y_LPF = in_array[lp_fifo_num]*lp_filter_coeff[0];
  
  for(int fir_counter = 1; fir_counter < N_FILTER_LENGTH; fir_counter++){
        int fir_index = lp_fifo_num - fir_counter;
      if (fir_index < 0) {
        fir_index += N_FILTER_LENGTH;
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
  
  LUT();
  index_test_signal = 0;
  setup_test_signal();
  Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(5000);
  
  //Serial.print("\n\rHello! Right now we are low pass filtering the input of DC PMT signal."); 
  //Serial.print("\n\r Correction factor: ");
  //Serial.print(correction_factor);
  //Serial.print("\n\r");
  delay(1000); 
  timer_setup();
}

void loop()
{  
  if(LP_Flag){

    //int temp;
    LP_Flag = false;
    after_LPF = execute_LPF();

    //temp = int(after_LPF);
    time = micros(); // the time in microseconds since the program was started

    Serial.print(time); Serial.print(",");
    Serial.print(after_LPF,5); Serial.print(",\n\r");

  //We will write our values as an array of bytes for easy
  //transmission and retrieval
  //unsigned char serialBytes[8];
  //serialBytes[0] = (time >> 24) & 0xff;
  //serialBytes[1] = (time >> 16) & 0xff;
  //serialBytes[2] = (time >> 8) & 0xff;
  //serialBytes[3] = time & 0xff;
  //serialBytes[4] = (temp >> 24) & 0xff;
  //serialBytes[5] = (temp >> 16) & 0xff;
  //serialBytes[6] = (temp >> 8) & 0xff;
  //serialBytes[7] = temp & 0xff;
  //Serial.write(serialBytes,8);
  }
  
  while(Serial.available())
  {    
    incomingByte = Serial.read();
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
        // Injection
        analogWrite(pwmPin1, RESOLUTION*(D_CYCLE1-83)/100);
        analogWrite(pwmPin2, RESOLUTION*(D_CYCLE2-83)/100);
        analogWrite(pwmPin3, RESOLUTION*(D_CYCLE3-94)/100);
        analogWrite(pwmPin4, RESOLUTION*D_CYCLE4/100);
        break;
      case 51:
        // Separation
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
