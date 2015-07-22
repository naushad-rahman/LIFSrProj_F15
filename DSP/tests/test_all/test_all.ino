//#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"
//#include "/Users/nordin/Documents/Arduino/libraries/ADC-master/ADC.h"

#define BAUD_RATE 115200
#define LENGTH_OF_LUT 10
#define LENGTH_OF_TEST_SIGNAL 1000
#define N_FILTER_LENGTH 37

// 1=raised cosine (calculated), 2=impulse, 3=raised cosine from lut, 4=rectangular pulse, 
// 5=rectangular modulation of raised cosine, 6=Gaussian modulation of raised cosine
#define SIMULATED_SIGNAL_SELECTION 1

#define OUTPUT_SAMPLE_INTERVAL 1
#define TIMER_INT_MICROS 1000
#define LENGTH_OF_DAC 10
#define LENGTH_OF_SIGNAL 37
#define N_FILTER_LENGTH 37
#define LENGTH_OF_TEST_SIGNAL 1000
#define BAUD_RATE 115200

float test_signal[LENGTH_OF_TEST_SIGNAL];
float in_array[LENGTH_OF_SIGNAL]; 
float after_BPF[LENGTH_OF_SIGNAL];
float after_cosmult[LENGTH_OF_SIGNAL];
float after_LPF[LENGTH_OF_SIGNAL];

IntervalTimer timer0;
float cosine_lut[LENGTH_OF_DAC]; 
int dac_lut[LENGTH_OF_DAC]; 
float twopi = 3.14159265359 * 2;

int index_in_array;
int index_cosmult_array;
int zero_phase_index;
int volatile index_test_signal;
boolean volatile data_ready_flag;

uint32_t counter;
uint32_t length_of_test_signal = LENGTH_OF_TEST_SIGNAL;
uint32_t length_of_lut = LENGTH_OF_LUT;

int n_mid_coef = (N_FILTER_LENGTH-1)/2;


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

void zero_array(float in[], int n_size) {
  for (int i = 1; i < n_size; i++) in[i] = 0.0;
}

//------------------------------------------------------------------
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
  counter = 0;
  // Set up buffers
  zero_array(in_array, N_FILTER_LENGTH);
  // Normalize bandpass filter coefficients
  normalize_coefficients(bp_filter_coeff, 0.0, N_FILTER_LENGTH);
  
  
  
  Serial.begin(BAUD_RATE);
  Serial.flush();
  delay(2000); 
}

void loop() {
  index_test_signal = counter % length_of_test_signal;
  index_in_array = counter % N_FILTER_LENGTH;
  in_array[index_in_array] = test_signal[index_test_signal];
  execute_FIR(in_array, after_BPF, bp_filter_coeff);
  
  zero_phase_index = (counter-n_mid_coef) % LENGTH_OF_SIGNAL;

   Serial.print(in_array[index_in_array]); Serial.print(",");
   Serial.print(after_BPF[index_in_array]); Serial.print(",\n\r");
  
  
  counter++;
}
