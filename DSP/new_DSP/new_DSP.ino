/*
Purpose: 1. Created a modulated signal from the DAQ to drive the laser
2. Do the DSP on the signal coming from the PMT
LIF Senior Project
*/
#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"

#define BAUD_RATE 115200
#define TIMER_INTER_MICRO 100
#define TAU_INTER_MICRO 100
#define CARRIER_FREQ 1e3
#define N_MOD 10
#define N_FILTER 32
#define ISR_counter_loop_reset 40 // 1 kHz sampling of the signal
#define LED_PIN 13



int daq_lut[N_MOD];
float adc_lut_i[N_MOD]; 
float adc_lut_q[N_MOD]; 

int volatile buf[N_FILTER]; 

volatile int n_sample = 0; 
float twopi = 3.14159265359 * 2;
int ISR_counter = 1;


IntervalTimer timer0;

//-------------------------------------------------------------------------

void LUT(){
  for(int lut_counter = 0; lut_counter < N_MOD; lut_counter++){
    daq_lut[lut_counter] = (int) (cos(twopi *CARRIER_FREQ * TAU_INTER_MICRO * 1e-6 * lut_counter)*2000.0 +2050.0); 
    adc_lut_i[lut_counter] =(cos(twopi *CARRIER_FREQ * TAU_INTER_MICRO * 1e-6 * lut_counter)); 
    adc_lut_q[lut_counter] = (-sin(twopi *CARRIER_FREQ * TAU_INTER_MICRO * 1e-6 * lut_counter)); 
  }
}//LUT

//---------------------------------------------------------------------------

void ISR(){
  // ISR counter reset and increment
  if (ISR_counter >= N_MOD){
    ISR_counter = 1; 
  }
  //Set DAC
  analogWrite(A14, daq_lut[ISR_counter]);
 
  if(n_sample > N_FILTER){
    n_sample = 0; 
  }
  
  buf[n_sample] = analogRead(A0); 
  
  ISR_counter++;
  n_sample++;
 
}// ISR

//---------------------------------------------------------------------------
void timer_setup() { 
  timer0.begin(ISR, TIMER_INTER_MICRO);  
} //timer_setup() 

//-------------------------------------------------------------------

void timer_stop() {
  timer0.end();
} //timer_stop() Teensy3

//------------------------------------------------------------------------

void setup() {
  analogWriteResolution(12); // Set up DAC resolution
  
  Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(5000); 
  LUT(); 
  Serial.print("Hello! Right now we are testing the DAC.\n"); 
  delay(1000); 
  timer_setup(); 
}//setup

//-----------------------------------------------------------------------

void loop() {
  // put your main code here, to run repeatedly:

}
