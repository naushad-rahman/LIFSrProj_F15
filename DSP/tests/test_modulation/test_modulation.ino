

#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"

#define BAUD_RATE 115200
#define TIMER_INT_MICROS 200
#define LENGTH_OF_DAC 10
#define LENGTH_OF_SIGNAL 37

#define N_FILTER_LENGTH 37

IntervalTimer timer0;
int dac_lut[LENGTH_OF_DAC]; 
float twopi = 3.14159265359 * 2;

void LUT(){
   for(int i = 0; i < LENGTH_OF_DAC; i++){
     dac_lut[i] = (int) ((cos(twopi*((float) i)/ LENGTH_OF_DAC) + 1)*2050); 
   Serial.print("\n\r dac_lut: ");
   Serial.print(dac_lut[i]);  
   }
}


void ISR(){




}

//--------------------------------------------------------------
void timer_setup() { //setup interrupts
  timer0.begin(ISR, TIMER_INT_MICROS);  
} //timer_setup() 

void setup() {
 analogWriteResolution(12); // Set up DAC resolution
 Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(5000); 
  LUT(); 
  Serial.print("\n\rHello! Right now we are testing the DSP with a fake pmt_signal.");

}

void loop() {
  // put your main code here, to run repeatedly:

}
