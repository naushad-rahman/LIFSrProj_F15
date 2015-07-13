/*
 * Use simulated data and apply bandpass filter with normalized coefficients and zero-phase.
 * 
 * Key variables:
 * 
 *   test_signal - array with simulated data. It is calculated in setup()
 *   in_array - buffer containing 37 samples consisting of the current and previous 36 raw samples
 *   after_bp - array containing the current and previous 36 samples passed through the bandpass filter
 *   counter - all other indices into arrays are calculated from this counter using the modulo (%) operation
 *   index_test_signal - index of current value from test_signal
 *   index_in_array - index of current value of in_array and after_bp
 *   zero_phase_index - index into in_array to get value corresponding to applying zero-phase filters
 *   current_value - value set in interrupt for the most recent raw sample
 *   data_ready_flag - true: a new value has been set in the interrupt
 *   
 */

//#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"
#include "/Users/nordin/Documents/Arduino/libraries/ADC-master/ADC.h"

// 1=raised cosine (calculated), 2=impulse, 3=raised cosine from lut, 4=rectangular pulse, 
// 5=rectangular modulation of raised cosine, 6=Gaussian modulation of raised cosine
#define SIMULATED_SIGNAL_SELECTION 5

#define OUTPUT_SAMPLE_INTERVAL 1
#define TIMER_INT_MICROS 800
#define LENGTH_OF_DAC 10
#define LENGTH_OF_SIGNAL 37
#define N_FILTER_LENGTH 37
#define LENGTH_OF_TEST_SIGNAL 1000
#define BAUD_RATE 115200

float test_signal[LENGTH_OF_TEST_SIGNAL];

float bp_filter_coeff[N_FILTER_LENGTH] = {
           0.000119, -0.000248, -0.001115, -0.002123, -0.002464, 
           -0.001282, 0.001674, 0.005529, 0.008370, 0.008080, 
           0.003597, -0.004103, -0.012010, -0.016287, -0.014199, 
           -0.005745, 0.005983, 0.016050, 0.020000, 0.016050, 
           0.005983, -0.005745, -0.014199, -0.016287, -0.012010, 
           -0.004103, 0.003597, 0.008080, 0.008370, 0.005529, 
           0.001674, -0.001282, -0.002464, -0.002123, -0.001115, -0.000248, 0.000119};

float lp_filter_coeff[N_FILTER_LENGTH] = {
           0.000326, 0.000692, 0.001207, 0.001891, 0.002756, 
           0.003806, 0.005036, 0.006430, 0.007961, 0.009591, 
           0.011275, 0.012958, 0.014582, 0.016086, 0.017413, 
           0.018507, 0.019325, 0.019829, 0.020000, 0.019829, 
           0.019325, 0.018507, 0.017413, 0.016086, 0.014582, 
           0.012958, 0.011275, 0.009591, 0.007961, 0.006430, 
           0.005036, 0.003806, 0.002756, 0.001891, 0.001207, 0.000692, 0.000326};

float in_array[LENGTH_OF_SIGNAL]; 
float after_BPF[LENGTH_OF_SIGNAL];
float after_cosmult[LENGTH_OF_SIGNAL];
float after_LPF[LENGTH_OF_SIGNAL];

IntervalTimer timer0;
float cosine_lut[LENGTH_OF_DAC]; 
int dac_lut[LENGTH_OF_DAC]; 
float twopi = 3.14159265359 * 2;

float volatile current_value;
int volatile index_test_signal;
boolean volatile data_ready_flag;

int index_in_array;
int zero_phase_index;
int counter;

int n_mid_coef = (N_FILTER_LENGTH-1)/2;

//-----------------------------------------------------------------
void cosine_LUT() {
   for(int i = 0; i < LENGTH_OF_DAC; i++) {
     cosine_lut[i] = cos(twopi*((float) i)/ LENGTH_OF_DAC);   
   }
}

//-----------------------------------------------------------------
void dac_LUT() {
   for(int i = 0; i < LENGTH_OF_DAC; i++) {
     dac_lut[i] = int ((1 + cosine_lut[i])*2047);   
   }
}

//-----------------------------------------------------------------
void execute_LPF() {
  after_LPF[index_in_array] = after_cosmult[index_in_array] * lp_filter_coeff[0];
  for (int fir_coef_index = 1; fir_coef_index < N_FILTER_LENGTH; fir_coef_index++) {
    int fir_delayline_index = index_in_array - fir_coef_index;
    if (fir_delayline_index < 0) {
      fir_delayline_index += N_FILTER_LENGTH;
    }
    after_LPF[index_in_array] += after_cosmult[fir_delayline_index] * lp_filter_coeff[fir_coef_index];
  }  
}

//-----------------------------------------------------------------
void execute_BPF() {
  after_BPF[index_in_array] = in_array[index_in_array] * bp_filter_coeff[0];
  for (int fir_coef_index = 1; fir_coef_index < N_FILTER_LENGTH; fir_coef_index++) {
    int fir_delayline_index = index_in_array - fir_coef_index;
    if (fir_delayline_index < 0) {
      fir_delayline_index += N_FILTER_LENGTH;
    }
    after_BPF[index_in_array] += in_array[fir_delayline_index] * bp_filter_coeff[fir_coef_index];
  }  
}

//-----------------------------------------------------------------
void execute_FIR(float in[], float out[], float h[]) {
  out[index_in_array] = in[index_in_array] * h[0];
  for (int fir_coef_index = 1; fir_coef_index < N_FILTER_LENGTH; fir_coef_index++) {
    int fir_delayline_index = index_in_array - fir_coef_index;
    if (fir_delayline_index < 0) {
      fir_delayline_index += N_FILTER_LENGTH;
    }
    out[index_in_array] += in[fir_delayline_index] * h[fir_coef_index];
  }  
}

//-----------------------------------------------------------------
void ISR() {
  // Use simulated PMT signal & apply bandpass filter
  current_value = test_signal[index_test_signal]; 
  data_ready_flag = true;
}

//-----------------------------------------------------------------
void timer_setup() { //setup interrupt
    timer0.begin(ISR, TIMER_INT_MICROS);  
} 

//-----------------------------------------------------------------
float gain_magn(float h[], float omega) {
    float cos_term = 0.0;
    float sin_term = 0.0;
    for (int k=0; k<N_FILTER_LENGTH; k++) {
        cos_term += h[k] * cos(twopi*omega*k);
        sin_term += h[k] * sin(twopi*omega*k);
    }
    float gain_mag = sqrt(cos_term*cos_term + sin_term*sin_term);
    return gain_mag;
}

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

void setup() {
  // Initialize look up tables
  cosine_LUT();
  dac_LUT();
  // Set up test signal
  setup_test_signal();
  // Initialize counters & flags
  //index_in_array = -1; 
  //index_test_signal = -1; 
  counter = 0;
  data_ready_flag = false;
  // Normalize bandpass filter coefficients
  float gain_mag = gain_magn(bp_filter_coeff, 0.1);
  for (int k=0; k<N_FILTER_LENGTH; k++) {
      bp_filter_coeff[k] /= gain_mag;
  }
  // Normalize low pass filter coefficients
  gain_mag = gain_magn(lp_filter_coeff, 0.0);
  for (int k=0; k<N_FILTER_LENGTH; k++) {
      lp_filter_coeff[k] /= gain_mag;
  }
  Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(1000); 
  timer_setup();
}

void loop() {
  int jj;
  if ( data_ready_flag && ((counter % OUTPUT_SAMPLE_INTERVAL) == 0) ) {
    // Calculate index variables
    index_in_array = counter % LENGTH_OF_SIGNAL;
    index_test_signal = counter % LENGTH_OF_TEST_SIGNAL;
    // Update delay line with latest sample from interrupt
    in_array[index_in_array] = current_value;
    // Apply bandpass filter
    execute_FIR(in_array, after_BPF, bp_filter_coeff);
    // Calculate index into in_array that corresponds to a zero-phase filter
    zero_phase_index = (counter-n_mid_coef) % LENGTH_OF_SIGNAL;
    //zero_phase_index = index_in_array - n_mid_coef;
    //if (zero_phase_index < 0) zero_phase_index += LENGTH_OF_SIGNAL;
    // Multiply band pass filtered data by cosine for demultiplexing
    jj = (counter-n_mid_coef - 1) % LENGTH_OF_DAC;
    for (int ii = 0; ii < N_FILTER_LENGTH; ii++) {
      int bp_index = index_in_array + ii;
      if (bp_index >= N_FILTER_LENGTH) bp_index -= N_FILTER_LENGTH;
      int lut_index = jj + ii;
      if (lut_index >= LENGTH_OF_DAC) lut_index -= LENGTH_OF_DAC;
      after_cosmult[bp_index] = after_BPF[bp_index] * cosine_lut[lut_index];
    }
    // Apply low pass filter
    execute_FIR(in_array, after_LPF, lp_filter_coeff);
    // Print values to serial port
    Serial.print(in_array[zero_phase_index]); Serial.print(",");
    Serial.print(after_BPF[index_in_array], 5); Serial.print(",");
    Serial.print(after_cosmult[index_in_array], 5); Serial.print(",");
    //Serial.print(cosine_lut[jj], 5); Serial.print(",");
    Serial.println(after_LPF[index_in_array], 5);
    data_ready_flag = false;
    counter++;
  }
}
