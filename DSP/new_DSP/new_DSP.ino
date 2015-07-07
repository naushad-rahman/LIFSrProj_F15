/*
Purpose: 1. Created a modulated signal from the DAQ to drive the laser
2. Do the DSP on the signal coming from the PMT
LIF Senior Project
*/
#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"

#define BAUD_RATE 115200
#define TIMER_INT_MICROS 100
#define LENGTH_OF_DAC 10
#define LENGTH_OF_SIGNAL 37

#define N_FILTER_LENGTH 37

int in_array[LENGTH_OF_SIGNAL]; 
float volatile after_BPF[LENGTH_OF_SIGNAL]; 
float volatile after_cos_mult[LENGTH_OF_SIGNAL];
float volatile after_LPF[LENGTH_OF_SIGNAL];


IntervalTimer timer0;
int dac_lut[LENGTH_OF_DAC]; 
float twopi = 3.14159265359 * 2;

int j; 
  int k; 

int volatile n_samp; 
int counter; 
bool volatile LP_Flag = false;
int LPF_counter;
float correction_factor; 

float bp_filter_coeff[N_FILTER_LENGTH] = {0.000119, -0.000248, -0.001115, -0.002123, -0.002464, 
           -0.001282, 0.001674, 0.005529, 0.008370, 0.008080, 
           0.003597, -0.004103, -0.012010, -0.016287, -0.014199, 
           -0.005745, 0.005983, 0.016050, 0.020000, 0.016050, 
           0.005983, -0.005745, -0.014199, -0.016287, -0.012010, 
           -0.004103, 0.003597, 0.008080, 0.008370, 0.005529, 
           0.001674, -0.001282, -0.002464, -0.002123, -0.001115, -0.000248, 0.000119};


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
  if(j >= LENGTH_OF_DAC){
    j = 0;
  }
  analogWrite(A14, dac_lut[j]); 
  
  //Fill up input fromm ADC
   in_array[k] = analogRead(A0);
   k++;
   if(k >= N_FILTER_LENGTH){
     k = 0;
      execute_BPF();
 
   }


  
 
  j++;
  n_samp++;
  
  // Set up ADC and Filter
  if(n_samp >= LENGTH_OF_SIGNAL){
    n_samp = 0; 
    LP_Flag = true; 
  }
}

//------------------------------------------------------------------------

void execute_BPF() {
  after_BPF[n_samp] = in_array[n_samp] * bp_filter_coeff[0];
  
  
  for(int fir_counter = 1; fir_counter < N_FILTER_LENGTH; fir_counter++){
      int fir_index = n_samp + fir_counter;
    if (fir_index >= N_FILTER_LENGTH) {
      fir_index -= N_FILTER_LENGTH;
    }
    after_BPF[n_samp] += in_array[fir_index] * bp_filter_coeff[fir_counter];
  //Serial.print("\n\r after_BPF: ");
  //Serial.print(after_BPF[n_samp]);
  }  
  //Serial.print("\n\r n_samp: ");
  //Serial.print(n_samp);
  //delay(1000);
  //Serial.print("\n\rafter_BPF: ");
  //Serial.print(after_BPF[n_samp]);
 

}

//-------------------------------------------------------------

void execute_demod(){
  float x_BPF[N_FILTER_LENGTH];
  int lut_index; 
  for(int i = 0; i < N_FILTER_LENGTH; i++){
      x_BPF[i] = after_BPF[i]; 
     // Serial.print("\n\r x_BPF: ");
      //Serial.print(x_BPF[i]);
  }
  
  for(int i = 0; i < N_FILTER_LENGTH; i++){
    lut_index = i%LENGTH_OF_DAC-8;
    if(lut_index < 0){
      lut_index += 10; 
    }
    after_cos_mult[i] = x_BPF[i] * dac_lut[lut_index];
   // Serial.print("\n\r after_cos_mult: ");
   // Serial.print(after_cos_mult[i]);
  }
}
//---------------------------------------

float execute_LPF(){
  int lp_fifo_num = 35; 
  
  float y_LPF = after_cos_mult[lp_fifo_num]*lp_filter_coeff[0];
 // Serial.print("\n\r after_cos_mult: ");
//  Serial.print(after_cos_mult[lp_fifo_num]);
  
  for(int fir_counter = 1; fir_counter < N_FILTER_LENGTH; fir_counter++){
        int fir_index = lp_fifo_num + fir_counter;
      if (fir_index >= N_FILTER_LENGTH) {
        fir_index -= N_FILTER_LENGTH;
      }
      y_LPF += after_cos_mult[fir_index] * lp_filter_coeff[fir_counter];
  
    }  
   //Serial.print("\n\r y_LPF: ");
   // Serial.print(y_LPF);
    
    return y_LPF/correction_factor; 



}


//--------------------------------------------------------------
void timer_setup() { //setup interrupts
  timer0.begin(ISR, TIMER_INT_MICROS);  
} //timer_setup() 

//---------------------------------------------------------------------
void setup() {
  analogWriteResolution(12); // Set up DAC resolution
  analogReadResolution(12); 
  
  j = 0; 
  k = 0; 
  n_samp = 0; 
  counter = 0; 
  LPF_counter = 0; 
  correction_factor = 0; 
  
  for(int i = 0; i < N_FILTER_LENGTH; i++){
    correction_factor += lp_filter_coeff[i]; 
  } 
  
  Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(5000); 
  LUT(); 
  Serial.print("\n\rHello! Right now we are testing the LPF.\n"); 
  Serial.print("\n\r Correction factor: ");
  Serial.print(correction_factor);
  delay(1000); 
  timer_setup(); 
}//setup

//-----------------------------------------------------------------------

void loop() {
 int average_out = 0; 

  if(LP_Flag){

    //int time = micros();
    execute_demod();
  //Serial.print("\n\r After_cos_mult: ");
  //Serial.print(after_cos_mult[n_samp]);

    after_LPF[LPF_counter] = execute_LPF();
    
//Serial.print("\n\r after_LPF: ");
//Serial.println(after_LPF[LPF_counter], 5);
  
    LPF_counter++; 
    if(LPF_counter >= N_FILTER_LENGTH){

      LPF_counter = 0; 
      for(int i = 0; i < N_FILTER_LENGTH; i++){
        average_out += after_LPF[i];
 
       }
    Serial.print("\n\r Average: "); 
    Serial.print(average_out);
      
    }

  }
  
}
