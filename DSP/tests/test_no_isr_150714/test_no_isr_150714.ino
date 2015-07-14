/*
 *   
 */

//#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"
#include "/Users/nordin/Documents/Arduino/libraries/ADC-master/ADC.h"

// 1=raised cosine (calculated), 2=impulse, 3=raised cosine from lut, 4=rectangular pulse, 
// 5=rectangular modulation of raised cosine, 6=Gaussian modulation of raised cosine
// 7=Gaussian
#define SIMULATED_SIGNAL_SELECTION 2

#define BAUD_RATE 115200
#define LENGTH_OF_LUT 10
#define LENGTH_OF_TEST_SIGNAL 1000
#define N_FILTER_LENGTH 37

float lp_filter_coeff[N_FILTER_LENGTH] = {
           0.000326, 0.000692, 0.001207, 0.001891, 0.002756, 
           0.003806, 0.005036, 0.006430, 0.007961, 0.009591, 
           0.011275, 0.012958, 0.014582, 0.016086, 0.017413, 
           0.018507, 0.019325, 0.019829, 0.020000, 0.019829, 
           0.019325, 0.018507, 0.017413, 0.016086, 0.014582, 
           0.012958, 0.011275, 0.009591, 0.007961, 0.006430, 
           0.005036, 0.003806, 0.002756, 0.001891, 0.001207, 0.000692, 0.000326};

float delay_line[N_FILTER_LENGTH]; 
float after_BPF[N_FILTER_LENGTH];
float after_cosmult[N_FILTER_LENGTH];
float after_LPF[N_FILTER_LENGTH];

uint32_t counter;
uint32_t length_of_test_signal = LENGTH_OF_TEST_SIGNAL;
uint32_t length_of_lut = LENGTH_OF_LUT;

int index_test_signal;
int index_delay_line;

float test_signal[LENGTH_OF_TEST_SIGNAL];

IntervalTimer timer0;
float cosine_lut[LENGTH_OF_LUT]; 
int dac_lut[LENGTH_OF_LUT]; 
float twopi = 3.14159265359 * 2;

//-----------------------------------------------------------------
void execute_FIR_linearphase(float in[], float out[], float h[], int ii) {
  out[ii] = in[ii] * h[0];
  for (int fir_coef_index = 1; fir_coef_index < N_FILTER_LENGTH; fir_coef_index++) {
    int fir_delayline_index = ii - fir_coef_index;
    if (fir_delayline_index < 0) fir_delayline_index += N_FILTER_LENGTH;
    out[ii] += in[fir_delayline_index] * h[fir_coef_index];
  }  
}

void zero_array(float in[], int n_size) {
  for (int i = 1; i < n_size; i++) in[i] = 0.0;
}

//-----------------------------------------------------------------
float gain_magn(float h[], float omega, int n_size) {
    float cos_term = 0.0;
    float sin_term = 0.0;
    for (int k=0; k<n_size; k++) {
        cos_term += h[k] * cos(twopi*omega*k);
        sin_term += h[k] * sin(twopi*omega*k);
    }
    float gain_mag = sqrt(cos_term*cos_term + sin_term*sin_term);
    return gain_mag;
}

//-----------------------------------------------------------------
void normalize_coefficients(float h[], float omega, int n_size) {
  float gain_mag = gain_magn(h, 0.0, n_size);
  for (int k=0; k<n_size; k++) {
      lp_filter_coeff[k] /= gain_mag;
  }

}

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
      mid = length_of_test_signal/2; width = length_of_test_signal/8;
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
    case 7: // Gaussian, range [0,1]
      mid = length_of_test_signal/2; width = length_of_test_signal/8;
      for (int ii = 0; ii < length_of_test_signal; ii++) {
        test_signal[ii] = exp( -pow((ii-mid),2.0)/(2.0*pow(width,2.0)) );
      }
      break;
    default: 
      break;
   }  
}

void setup() {
  counter = 0;
  setup_test_signal();
  zero_array(delay_line, N_FILTER_LENGTH);
  zero_array(after_LPF, N_FILTER_LENGTH);
  // Normalize low pass filter coefficients
  normalize_coefficients(lp_filter_coeff, 0.0, N_FILTER_LENGTH);
  Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(500);
}

void loop() {
  index_test_signal = counter % length_of_test_signal;
  index_delay_line = counter % N_FILTER_LENGTH;
  delay_line[index_delay_line] = test_signal[index_test_signal];
  execute_FIR_linearphase(delay_line, after_LPF, lp_filter_coeff, index_delay_line);
  Serial.print(index_test_signal); Serial.print(",");
  Serial.print(index_delay_line); Serial.print(",");
  Serial.print(delay_line[index_delay_line], 5); Serial.print(",");
  Serial.println(after_LPF[index_delay_line], 5);
  counter++;
  if (counter == 100000) counter = 0;
  delayMicroseconds(500); // wait for specified number of microseconds
}
