/* 
This code takes samples from the PMT and the perform the DSP
Teensy 3.1
*/

//#include <ADC.h>

#define TimerInterruptInMicroSeconds 25 // This is for 40 kHz
#define CarrierFrequency 1000

// Can go from 10 to 20 here, from 2 kHz sampling to 4 kHz sampling
#define ISR_counter_loop_reset 40 // 1 kHz sampling of the signal

int ISR_counter = 1;

#define MaxNumSamples 0  //set to 0 if want continuous sampling (i.e. run forever)
#define NumSamplesAverageInADC 32 //hardware averaging default is 2. values 0..32. Higher values reduce noise and maximum sample rate
#define NumADCbits 16


int CyclesInState = 0;
#define StateSwitchingCycles 100
// While 32 is the minimum, it seems to take a few cycles to get things to work right. 32 + 18 + 18 + 18
#define PreCalculateCycles 90
#define NumCalculatedAverages 10

#define BAUD_RATE 921600
#define HW_BAUD_RATE 115200


volatile boolean LEDon = true;
volatile unsigned long counter = 0; //keep track of how many times the timer interrupt executes
elapsedMillis sinceStartmillis;
elapsedMicros sinceStartmicros;

volatile int phase_counter = 0;
float twopi = 3.14159265359 * 2;

#define pre_compute_length 4000
float i_mod_pre[pre_compute_length];
float q_mod_pre[pre_compute_length];
int out_val_pre[pre_compute_length]; 

float current_i_mod;
float current_q_mod;


#define bp_fir_length 78 
#define lp_fir_length 32

float x_0[bp_fir_length];
float x_1[bp_fir_length];

float x_0_i[lp_fir_length];
float x_0_q[lp_fir_length];
float x_1_i[lp_fir_length];
float x_1_q[lp_fir_length];


float real_ip = 0;
float real_ip_total = 0;
float imag_ip = 0;
float imag_ip_total = 0;
float y_0_squared = 0;
float y_1_squared = 0;


int max_0 = -1;
int min_0 = 65536;
int max_1 = -1;
int min_1 = 65536;
//FDAtool Bandpass parameters
// Fir band pass filter: Chebysheve window
// Fs =10000
// Fc1 = 950
// Fc2 = 1050
// fdatool in matlab
float bp_filter_coeff[bp_fir_length] = {1.21543647198309e-05,-3.53040593613539e-20,-4.55045639556942e-05,-0.000130102411776067,-0.000215240232502404,-0.000209311169231389,
4.47226036556179e-19,0.000462454466203243,0.00106302101679958,0.00147305212991216,0.00123402401621121,-1.78704951086492e-18,-0.00214021966146080,-0.00444547033759249,
-0.00562011655369128,-0.00432907323704203,5.13037709956765e-18,0.00646532935475108,0.0125532664870180,0.0148936821829106,0.0108030422337513,-7.79294021228872e-18,-0.0144283732255282,
-0.0265867713720006,-0.0299957643764942,-0.0207261620124529 ,6.95592273032741e-18,0.0252335182713757,0.0444788678176698,0.0480560831607591,0.0318292099055091,-2.01597387989092e-17,
-0.0356930749999310,-0.0604441753328635,-0.0627752495902097,-0.0399865979622131,0,0.0415207259816352,0.0676888545104299,0.0676888545104299,0.0415207259816352,0,-0.0399865979622131,-0.0627752495902097,
-0.0604441753328635,-0.0356930749999310,-2.01597387989092e-17,0.0318292099055091,0.0480560831607591,0.0444788678176698,0.0252335182713757,6.95592273032741e-18,-0.0207261620124529,-0.0299957643764942,
-0.0265867713720006,-0.0144283732255282,-7.79294021228872e-18,0.0108030422337513,0.0148936821829106,0.0125532664870180,0.00646532935475108,5.13037709956765e-18,-0.00432907323704203,-0.00562011655369128,
-0.00444547033759249,-0.00214021966146080,-1.78704951086492e-18,0.00123402401621121,0.00147305212991216,0.00106302101679958,0.000462454466203243,4.47226036556179e-19,-0.000209311169231389,-0.000215240232502404,
-0.000130102411776067,-4.55045639556942e-05,-3.53040593613539e-20,1.21543647198309e-05
};



// Coefficients for FIR low-pass filter
// Order: 31
// Fs = 1000
// Fpass = 5
// Fstop = 30
// Density Factor 20
float lp_filter_coeff[lp_fir_length] = {
0.057543154805898666,
0.015798510983586311,
0.017765788361430168,
0.019689911976456642,
0.021637413650751114,
0.023519175127148628,
0.025364236906170845,
0.02709488570690155 ,
0.028727615252137184,
0.03017670102417469 ,
0.031480800360441208,
0.032562818378210068,
0.033506937325000763,
0.034178838133811951,
0.034685637801885605,
0.034876484423875809,
0.034876484423875809,
0.034685637801885605,
0.034178838133811951,
0.033506937325000763,
0.032562818378210068,
0.031480800360441208,
0.03017670102417469 ,
0.028727615252137184,
0.02709488570690155 ,
0.025364236906170845,
0.023519175127148628,
0.021637413650751114,
0.019689911976456642,
0.017765788361430168,
0.015798510983586311,
0.057543154805898666
};



int bp_fifo_num = 1;
int lp_fifo_num = 1;



int value[] = {0,0};


float mag = 0;
float calc_mag = 0;
float mag_total = 0;
float calc_phase = 0;
int NumInBlock = 0;

float mag_result[3] = {0, 0, 0};
float phase_result[3] = {0, 0, 0};
int diff_0[3] = {0, 0, 0};
int diff_1[3] = {0, 0, 0};

float final_mag = 0;
float final_phase = 0;

// Finite State Machine
int current_state = 0;


// Flag
boolean ADC_reading = false;

// This is necessary for the ADC library
ADC *adc = new ADC(); // adc object

IntervalTimer timer0;

// For sending bytes
char * b;

//Modulatioin and Demodulation
void pre_compute_tables() {
  // This function will precompute the cos and sin tables used in the rest of the program
  for(int precompute_counter = 0; precompute_counter < pre_compute_length; precompute_counter++){
    out_val_pre[precompute_counter] = (int) (cos(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter) * 2000.0 + 2050.0);
    i_mod_pre[precompute_counter] = (cos(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter));
    q_mod_pre[precompute_counter] = (-sin(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter));

  }
} //pre_compute_tables


void execute_DSP() {
  bp_fifo_num++;  
  if (bp_fifo_num >= bp_fir_length) bp_fifo_num = 0;

   x_0[bp_fifo_num] = (float) (value[1] - value[0]);//Voltage divider
   x_1[bp_fifo_num] = (float) (value[0] - 32768);  // 3278 is half a 12 bit ADC

   // Band pass filter of incoming data
  float y_0 = x_0[bp_fifo_num] * bp_filter_coeff[0];
  float y_1 = x_1[bp_fifo_num] * bp_filter_coeff[1];
  
  for(int fir_counter = 1; fir_counter < bp_fir_length; fir_counter++){
    int fir_index = bp_fifo_num + fir_counter;
    if (fir_index >= bp_fir_length) {
      fir_index -= bp_fir_length;
    }
    y_0 += x_0[fir_index] * bp_filter_coeff[fir_counter];
    y_1 += x_1[fir_index] * bp_filter_coeff[fir_counter];
  }

  current_i_mod = i_mod_pre[phase_counter];
  current_q_mod = q_mod_pre[phase_counter];

  // Demodulation of the signal
  x_0_i[lp_fifo_num] = (y_0 * current_i_mod);
  x_0_q[lp_fifo_num] = (y_0 * current_q_mod);
  x_1_i[lp_fifo_num] = (y_1 * current_i_mod);
  x_1_q[lp_fifo_num] = (y_1 * current_q_mod); 

  lp_fifo_num++;  
  if (lp_fifo_num >= lp_fir_length) lp_fifo_num = 0;//circular buffer

  // Low pass filter of demodulated signal
  float y_0_i = x_0_i[lp_fifo_num] * lp_filter_coeff[0];
  float y_0_q = x_0_q[lp_fifo_num] * lp_filter_coeff[0];
  float y_1_i = x_1_i[lp_fifo_num] * lp_filter_coeff[0];
  float y_1_q = x_1_q[lp_fifo_num] * lp_filter_coeff[0];

  for(int fir_counter = 1; fir_counter < lp_fir_length; fir_counter++){
    int fir_index = lp_fifo_num + fir_counter;
    if (fir_index >= lp_fir_length) {
      fir_index -= lp_fir_length;
    }
    y_0_i += x_0_i[fir_index] * lp_filter_coeff[fir_counter];
    y_0_q += x_0_q[fir_index] * lp_filter_coeff[fir_counter];
    y_1_i += x_1_i[fir_index] * lp_filter_coeff[fir_counter];
    y_1_q += x_1_q[fir_index] * lp_filter_coeff[fir_counter];
  }  
   
  real_ip = y_0_i * y_1_i + y_0_q * y_1_q; //Square coordinates
  imag_ip = y_0_q * y_1_i - y_0_i * y_1_q;
  y_0_squared = y_0_i * y_0_i + y_0_q * y_0_q; 
  y_1_squared = y_1_i * y_1_i + y_1_q * y_1_q;

//  float mag = sqrt((real_ip * real_ip + imag_ip * imag_ip) / y_0_squared;
//sqart(x^2+yj^2)
  mag = sqrt(((float) real_ip * (float) real_ip + (float) imag_ip * (float) imag_ip)) / ((float) y_0_squared);
  mag_total += mag;
  calc_mag = mag_total / NumInBlock;

  real_ip_total += real_ip;
  imag_ip_total += imag_ip;
  calc_phase = -atan2(imag_ip_total, real_ip_total);
}//execute DSP


void ISR_repeat() {
  // Calculation of loop for generating signal to output on DAC
  analogWrite(A14, out_val_pre[phase_counter]);//outputs sine wave at carrier frequency

  // ISR counter reset and increment
  if (ISR_counter >= ISR_counter_loop_reset)
  {
    ISR_counter = 1;
  //    adc->startSingleDifferential(A10,A11, ADC_0); // These lines get the measurements started
  //    adc->startSingleDifferential(A12,A13, ADC_1);
    adc->startSingleRead(A1, ADC_0);
    adc->startSingleRead(A3, ADC_1);
    ADC_reading = true;
    current_i_mod = i_mod_pre[phase_counter];
    current_q_mod = q_mod_pre[phase_counter];
  }
  else {
    ISR_counter++;
  }
  phase_counter++;
  if (phase_counter >= pre_compute_length) phase_counter = 0;
  

//For their cart
  // Now check the digital distance counter
  int val = digitalRead(DistanceCounterPIN);
  if (val != DistanceCountState) {
    DistanceCount++;
    DistanceCountState = val;
  }
  
  
} //ISR_repeat


void loop(){
  pre_compute_tables();
  for(int i = 0; i < pre_compute_length; i++{
    Serial.print(out_val_pre[]);
  };
  



}
