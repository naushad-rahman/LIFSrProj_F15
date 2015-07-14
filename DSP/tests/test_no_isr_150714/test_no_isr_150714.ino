/*
 *   
 */

//#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"
#include "/Users/nordin/Documents/Arduino/libraries/ADC-master/ADC.h"

// 1=raised cosine (calculated), 2=impulse, 3=raised cosine from lut, 4=rectangular pulse, 
// 5=rectangular modulation of raised cosine, 6=Gaussian modulation of raised cosine
#define SIMULATED_SIGNAL_SELECTION 2

#define BAUD_RATE 115200
#define LENGTH_OF_LUT 10
#define LENGTH_OF_TEST_SIGNAL 1000

uint32_t counter;
uint32_t length_of_test_signal = LENGTH_OF_TEST_SIGNAL;
uint32_t length_of_lut = LENGTH_OF_LUT;
uint32_t index_test_signal;

float test_signal[LENGTH_OF_TEST_SIGNAL];

IntervalTimer timer0;
float cosine_lut[LENGTH_OF_LUT]; 
int dac_lut[LENGTH_OF_LUT]; 
float twopi = 3.14159265359 * 2;

//-----------------------------------------------------------------
void setup_test_signal() {
  int mid, width;
  switch (SIMULATED_SIGNAL_SELECTION) {
    case 1: // raised cosine as float in range [0,2]
      for (int ii = 0; ii < length_of_test_signal; ii++) {
        test_signal[ii] = 1.0 + cos(twopi*((float) ii)/ length_of_lut);
      }
      break;
    case 2: // impulse with range [0,1]
      for (int ii=0; ii<length_of_test_signal; ii++) test_signal[ii] = 0.0;
      test_signal[length_of_test_signal/2] = 1.0;
      break;
    case 3: // raised cosine using look up table, range is [0,4094]
      for (int ii=0; ii<length_of_test_signal; ii++) {
        test_signal[ii] = dac_lut[ii % length_of_lut];
      }
      break;
    case 4: // rectangular pulse with range [0,1]
      mid = length_of_test_signal/2; width = length_of_test_signal/4;
      for (int ii=0; ii<length_of_test_signal; ii++) test_signal[ii] = 0.0;
      for (int ii=mid-width/2; ii<mid+width/2; ii++) test_signal[ii] = 1.0;
      break;
    case 5: // rectangular modulation of raised cosine, range [0,2]
      mid = length_of_test_signal/2; width = length_of_test_signal/4;
      for (int ii=0; ii<length_of_test_signal; ii++) test_signal[ii] = 0.0;
      for (int ii=mid-width/2; ii<mid+width/2; ii++) test_signal[ii] = 1.0;
      for (int ii = 0; ii < length_of_test_signal; ii++) {
        test_signal[ii] *= 1.0 + cos(twopi*((float) ii)/ length_of_lut);
      }
      break;
    case 6: // Gaussian modulation of raised cosine, range [0,2]
      mid = length_of_test_signal/2; width = length_of_test_signal/8;
      for (int ii = 0; ii < length_of_test_signal; ii++) {
        test_signal[ii] = exp( -pow((ii-mid),2.0)/(2.0*pow(width,2.0)) );
        test_signal[ii] *= (1.0 + cos(twopi*((float) ii)/ length_of_lut));
      }
      break;
    default: 
      break;
   }  
}

void setup() {
  counter = 0;
  setup_test_signal();
  Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(500);
}

void loop() {
  index_test_signal = counter % length_of_test_signal;
  uint32_t tempint = length_of_test_signal;
  Serial.print(counter); Serial.print(",");
  Serial.print(tempint); Serial.print(",");
  Serial.print(index_test_signal); Serial.print(",");
  Serial.println(test_signal[index_test_signal]);
  counter++;
  if (counter == 100000) counter = 0;
  delayMicroseconds(500); // wait for specified number of microseconds
}
