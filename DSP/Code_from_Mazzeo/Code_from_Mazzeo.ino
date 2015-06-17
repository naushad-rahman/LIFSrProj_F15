/* OVERVIEW
 Purpose: This samples the two voltages and then performs the DSP to caclulate magnitude and phase
 Microcontroller: Teensy 3.1
  Brian Mazzeo
  6/4/2014 
*/

#include <ADC.h>


// Definitions
//#define TimerInterruptInMicroSeconds 100 // This is for 100 kHz
//#define TimerInterruptInMicroSeconds 50 // This is for 20 kHz
#define TimerInterruptInMicroSeconds 25 // This is for 40 kHz


#define CarrierFrequency 190 // in hertz

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

// Coefficients for FIR band-pass filter (single-precision floating)
// Order: 31
// Fs = 1000
// Fstop1 = 140
// Fpass1 = 180
// Fpass2 = 200
// Fstop2 = 240  
// Density Factor 20
//fda tool in matlab
float bp_filter_coeff[bp_fir_length] = {
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
}


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

void timer_setup() { //setup interrupts
  timer0.begin(ISR_repeat, TimerInterruptInMicroSeconds);  
} //timer_setup() 
  
void timer_stop() {
  timer0.end();
} //timer_stop() Teensy3


void setup() {
  pinMode(LED_PIN, OUTPUT);
//  analogReference(DEFAULT); //range 0..3.3v - note this voltage regulator is noisy
//  analogReference(EXTERNAL);

// Analog inputs  
    pinMode(A1, INPUT);
    pinMode(A3, INPUT);

// Digital outputs to switch resistors  
//Mux  
    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);

// Digital input for counter  
// not needed  
    pinMode(DistanceCounterPIN, INPUT);
    DistanceCountState = digitalRead(DistanceCounterPIN);

//  adc->setReference(ADC_REF_INTERNAL, ADC_0); // change all 3.3 to 1.2 if you change the reference
//  adc->setReference(ADC_REF_INTERNAL, ADC_1);

  adc->setReference(ADC_REF_EXTERNAL, ADC_0); // change all 3.3 to 1.2 if you change the reference
  adc->setReference(ADC_REF_EXTERNAL, ADC_1);

  
  //adc->enablePGA(4, ADC_0);
  adc->setAveraging(32, ADC_0); // set number of averages
  adc->setResolution(16, ADC_0); // set bits of resolution

  //adc->enablePGA(5, ADC_1);
  adc->setAveraging(32, ADC_1); // set number of averages
  adc->setResolution(16, ADC_1); // set bits of resolution

  // always call the compare functions after changing the resolution!
  //adc->enableCompare(1.0/3.3*adc->getMaxValue(ADC_0), 0, ADC_0); // measurement will be ready if value < 1.0V
  //adc->enableCompareRange(1.0*adc->getMaxValue(ADC_1)/3.3, 2.0*adc->getMaxValue(ADC_1)/3.3, 0, 1, ADC_1); // ready if value lies out of [1.0,2.0] V

  analogWriteResolution(12); // Set up DAC resolution
  
//  analogReadResolution(NumADCbits); //try 12 bits on analog reads. Should be able to do 16 bits on A10 & A11
//  analogReadAveraging(NumSamplesAverageInADC);
  Serial2.begin(HW_BAUD_RATE);
  Serial2.flush();


  Serial.begin(BAUD_RATE);
  delay(5000);  //Give some time to start terminal on computer to catch serial data
  pre_compute_tables();
  Serial.flush();
  Serial.print("Hello! This is the DAQ Signal Processing program.\n");
  delay(1000);
  sinceStartmillis = 0;
  timer_setup(); //turn on timer  
} //setup


//Main
// This loop happens every ms or at a 1 kHz rate
void loop() {
  while (!ADC_reading) {
    digitalWrite(LED_PIN, LOW);
  }
  digitalWrite(LED_PIN, HIGH);

  counter++;
  
  CyclesInState++;
  switch (current_state) {
    case 0:
          // 10 MOhm resistor - S4
          digitalWrite(D0, HIGH);
          digitalWrite(D1, HIGH);
          digitalWrite(D2, LOW);
          break;
    case 1:
          // 100 kOhm resistor - S3
          digitalWrite(D0, LOW);
          digitalWrite(D1, HIGH);
          digitalWrite(D2, LOW);
          break;
    case 2:
          // 910 Ohm resistor + 100 Ohm on resistance - S2
          digitalWrite(D0, HIGH);
          digitalWrite(D1, LOW);
          digitalWrite(D2, LOW);
          break;
    default:
          digitalWrite(D0, LOW);
          digitalWrite(D1, LOW);
          digitalWrite(D2, LOW);
          break;
  }

  execute_DSP();
  
  if (CyclesInState <= PreCalculateCycles) {
    NumInBlock = 0;
    mag_total = 0;
    real_ip_total = 0;
    imag_ip_total = 0;
    max_0 = -1;
    max_1 = -1;
    min_0 = 65536;
    min_1 = 65536;
  }
  else {
    NumInBlock++;
    if (NumInBlock == NumCalculatedAverages) {
      mag_result[current_state] = calc_mag;
      phase_result[current_state] = calc_phase;
      NumInBlock = 0;
      mag_total = 0;
      real_ip_total = 0;
      imag_ip_total = 0;
    }
  }
  
  if (CyclesInState == StateSwitchingCycles) {   
    CyclesInState = 0;
    switch (current_state) {
      case 0:
          current_state = 1;
          break;
      case 1:
          current_state = 2;
          break;
      case 2:
          if (mag_result[0] >= 0.1) {
            final_mag = 10000000 * mag_result[0];
            final_phase = phase_result[0];
          }
          else if (mag_result[1] >= 0.1) {
            final_mag = 100000 * mag_result[1];
            final_phase = phase_result[1];
          }
          else {
            final_mag = 1000 * mag_result[2];
            final_phase = phase_result[2];
          }
//          Serial.print(counter, DEC);
//          Serial.write('\t');
//          Serial.print(DistanceCount, DEC);
//          Serial.write('\t');
//          Serial.print(final_mag, 8);
//          Serial.write('\t');
//          Serial.print(final_phase, 8);
//          Serial.write('\n');

          Serial2.print("D");
          Serial2.print(sinceStartmillis, DEC);
          Serial2.write('_');
          Serial2.print(DistanceCount, DEC);
          Serial2.write('_');
          Serial2.print(final_mag, 6);
          Serial2.write('_');
          Serial2.print(final_phase, 6);
          Serial2.write('\n');
          
          current_state = 0;
          break;
      default:
          current_state = 0;
    }
  }


  if ((counter >= MaxNumSamples) & (MaxNumSamples != 0)) timer_stop(); //Stop

  value[0] = adc->readSingle(ADC_0); // These lines then get the simultaneous reads of the measurements
  value[1] = adc->readSingle(ADC_1); 
  ADC_reading = false;
  value[0] &= 0x0000FFFF; // This is because for 16-bit reads, the program will automatically try to 2's complement the numbers
  value[1] &= 0x0000FFFF;
  

} //loop
