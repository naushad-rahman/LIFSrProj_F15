/* OVERVIEW
 Purpose: This sampes the voltage from the PMT and then performs the DSP
  Senior Project: LIF
  6/15/2015
*/

#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"


// Definitions
//#define TimerInterruptInMicroSeconds 100 // This is for 100 kHz
//#define TimerInterruptInMicroSeconds 50 // This is for 20 kHz
#define TimerInterruptInMicroSeconds 25 // This is for 40 kHz


#define CarrierFrequency 1e3

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
//#define NumCalculatedAverages 500

//#define BAUD_RATE 921600  //921600 //460800 // 230400 //115200
//#define BAUD_RATE 921600
#define BAUD_RATE 921600
#define HW_BAUD_RATE 115200


#define LED_PIN 13
#define D0 0
#define D1 1
#define D2 2
#define DistanceCounterPIN 4
volatile unsigned long DistanceCount = 0;
volatile int DistanceCountState = 0;

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

#define bp_fir_length 32  
#define lp_fir_length 32

float x_0[bp_fir_length];

float x_0_i[lp_fir_length];
float x_0_q[lp_fir_length];


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

#define BUFFER_LENGTH 100

float volatile buffer_in[BUFFER_LENGTH];
float volatile *p; 
int time; 
int ptr_index;

// Coefficients for FIR band-pass filter (single-precision floating)
// Order: 31
// Fs = 1000
// Fstop1 = 140
// Fpass1 = 180
// Fpass2 = 200
// Fstop2 = 240  
// Density Factor 20
float bp_filter_coeff[bp_fir_length] = /*{
-0.0068805036135017872,
 0.0061070485971868038,
-0.02114810049533844  ,
-0.018354915082454681 ,
 0.016083890572190285 ,
 0.045424636453390121 ,
 0.019806263968348503 ,
-0.044957991689443588 ,
-0.062694497406482697 ,
 0.0094339223578572273,
 0.089177675545215607 ,
 0.06248362734913826  ,
-0.052480928599834442 ,
-0.10870025306940079  ,
-0.023628279566764832 ,
 0.098564200103282928 ,
 0.098564200103282928 ,
-0.023628279566764832 ,
-0.10870025306940079  ,
-0.052480928599834442 ,
 0.06248362734913826  ,
 0.089177675545215607 ,
 0.0094339223578572273,
-0.062694497406482697 ,
-0.044957991689443588 ,
 0.019806263968348503 ,
 0.045424636453390121 ,
 0.016083890572190285 ,
-0.018354915082454681 ,
-0.02114810049533844  ,
 0.0061070485971868038,
-0.0068805036135017872
};*/
//FDAtool Bandpass parameters
//Order = 31
// Fs =2100
// Fstop1 = 930
// Fpass1 = 990
// Fpass2 = 1010
// Fstop2 = 1050
//Density = 20
// fdatool in matlab
{
-0.0385626741831516,
0.0243321731032428,
-0.0306418781682252,
0.0368656499455263,
-0.0426333194742660,
0.0475586702729681,
-0.0512993260783385,
0.0535042653011438,
-0.0539034139355538,
0.0523335298254837,
-0.0487363284803318,
0.0431377288844155,
-0.0356877313177569,
0.0267196488109811,
-0.0165171825506038,
0.00559602625109013,
0.00559602625109013,
-0.0165171825506038,
0.0267196488109811,
-0.0356877313177569,
0.0431377288844155,
-0.0487363284803318,
0.0523335298254837,
-0.0539034139355538,
0.0535042653011438,
-0.0512993260783385,
0.0475586702729681,
-0.0426333194742660,
0.0368656499455263,
-0.0306418781682252,
0.0243321731032428,
-0.0385626741831516};


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



IntervalTimer timer0;

// For sending bytes
char * b;

void pre_compute_tables() {
  // This function will precompute the cos and sin tables used in the rest of the program
  for(int precompute_counter = 0; precompute_counter < pre_compute_length; precompute_counter++){
    out_val_pre[precompute_counter] = (int) (cos(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter) * 2000.0 + 2050.0);
    i_mod_pre[precompute_counter] = (cos(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter));
    q_mod_pre[precompute_counter] = (-sin(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter));

  }
} //pre_compute_tables


float execute_BPF(float input_signal) {

  if (bp_fifo_num >= bp_fir_length) bp_fifo_num = 0;

   x_0[bp_fifo_num] = input_signal;
   bp_fifo_num++; 
  
   // Band pass filter of incoming data
  float y_0 = x_0[bp_fifo_num] * bp_filter_coeff[0];
  
  for(int fir_counter = 1; fir_counter < bp_fir_length; fir_counter++){
      int fir_index = bp_fifo_num + fir_counter;
    if (fir_index >= bp_fir_length) {
      fir_index -= bp_fir_length;
    }

    y_0 += x_0[fir_index] * bp_filter_coeff[fir_counter];

  } 
  
  return y_0; 

  
}


float execute_LPF(float y){
  current_i_mod = i_mod_pre[phase_counter];
  current_q_mod = q_mod_pre[phase_counter];
  
  // Demodulation of the signal
  x_0_i[lp_fifo_num] = (y * current_i_mod);
  x_0_q[lp_fifo_num] = (y * current_q_mod); 

  if (lp_fifo_num >= lp_fir_length) lp_fifo_num = 0;
  lp_fifo_num++;  

  // Low pass filter of demodulated signal
  float y_0_i = x_0_i[lp_fifo_num] * lp_filter_coeff[0];
  float y_0_q = x_0_q[lp_fifo_num] * lp_filter_coeff[0];
 
  for(int fir_counter = 1; fir_counter < lp_fir_length; fir_counter++){
    int fir_index = lp_fifo_num + fir_counter;
    if (fir_index >= lp_fir_length) {
      fir_index -= lp_fir_length;
    }
    y_0_i += x_0_i[fir_index] * lp_filter_coeff[fir_counter];
    y_0_q += x_0_q[fir_index] * lp_filter_coeff[fir_counter];
  }  
   
  //Serial.print("\n\r y_i: ");
  //Serial.println(y_0_i, 8);
  //Serial.print("\r y_q: ");
  //Serial.println(y_0_q, 8);
  
 
  y_0_squared = y_0_i * y_0_i + y_0_q * y_0_q; 
  
  *(ptr_index+p) = y_0_squared; 

  ptr_index++; 
  

  return y_0_squared;
  
 

}

float signal;
float signal_bpf;

void ISR() {
  
  signal = analogRead(A0); //Sample rate is 10kHz (Documentation online) 
  signal_bpf = execute_BPF(signal);
  execute_LPF(signal_bpf);
  
  Serial.print("\r\n");
  Serial.println("signal: ");
  Serial.println(signal,5);

  // ISR counter reset and increment
  if (ISR_counter >= ISR_counter_loop_reset)
  {
    ISR_counter = 1; 
    current_i_mod = i_mod_pre[phase_counter];
    current_q_mod = q_mod_pre[phase_counter];
  }
  else {
    ISR_counter++;
  }
  
  phase_counter++;
  if (phase_counter >= pre_compute_length) phase_counter = 0;
  
  // Calculation of loop for generating signal to output on DAC
  analogWrite(A14, out_val_pre[phase_counter]);

} //ISR

void timer_setup() { //setup interrupts
  timer0.begin(ISR, TimerInterruptInMicroSeconds);  
} //timer_setup() 
  
void timer_stop() {
  timer0.end();
} //timer_stop() Teensy3


void setup() {
  pinMode(LED_PIN, OUTPUT);

// Analog inputs  
  pinMode(A0, INPUT);
  
  p = buffer_in; 
  ptr_index = 0; 
 
  analogWriteResolution(12); // Set up DAC resolution
  
  analogReadResolution(NumADCbits); //try 12 bits on analog reads. Should be able to do 16 bits on A10 & A11
  Serial2.begin(HW_BAUD_RATE);
  Serial2.flush();


  delay(5000);  //Give some time to start terminal on computer to catch serial data
  pre_compute_tables();
  Serial.print("Hello! This is the DAQ and DSP program.\n");
  delay(1000);
  sinceStartmillis = 0;
  timer_setup(); //turn on timer  
} //setup




bool first = true;

int coefficient_count = 31;

void runTest(){
//Test with Dirac delta function
   if(first){
    signal = 1.0;
    first = false;
  }else{
    signal = 0.0;
  }

  signal_bpf = execute_BPF(signal);
  execute_LPF(signal_bpf);
  
  if(coefficient_count-- == 0){
    delay(1000000);
  }

}

// This loop happens every ms or at a 1 kHz rate
void loop() {
  float sum = 0; 
  int average = 0; 
  if(ptr_index > (BUFFER_LENGTH -1) ){
      ptr_index = 0; 
      for(int i = 0; i < BUFFER_LENGTH; i++){
         sum += buffer_in[i]; 
      }
      average = int(sum); 
      time = micros();
      
    unsigned char serialBytes[8]; 
    serialBytes[0] = (time >> 24) & 0xff;
    serialBytes[1] = (time >> 16) & 0xff;
    serialBytes[2] = (time >> 8) & 0xff; 
    serialBytes[3] = time & 0xff; 
    serialBytes[4] = (average >> 24) & 0xff;
    serialBytes[5] = (average >> 16) & 0xff;
    serialBytes[6] = (average >> 8) & 0xff; 
    serialBytes[7] = average & 0xff; 
    Serial.write(serialBytes,8); 
  
  }
  
} //loop

