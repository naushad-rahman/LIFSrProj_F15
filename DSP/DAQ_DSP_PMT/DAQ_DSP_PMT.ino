


#include "/home/samuelwg/Arduino/libraries/ADC/ADC-master/ADC.h"


//DAC and ADC Interupt
#define TimerInterruptInMicroSeconds 25 // This is for 40 kHz
#define CarrierFrequency 1e3 
#define ISR_counter_loop_reset 40 // 1 kHz sampling of the signal

IntervalTimer timer0;


#define HW_BAUD_RATE 115200


#define bp_fir_length 78 
#define lp_fir_length 32

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

int ISR_counter = 1;
volatile int phase_counter = 0; 
float twopi = 3.14159265359 * 2;

#define pre_compute_length 4000
float i_mod_pre[pre_compute_length];
float q_mod_pre[pre_compute_length];
int out_val_pre[pre_compute_length];

float current_i_mod;
float current_q_mod;

//Bandpass filter characteristics
float x_0[bp_fir_length];
int bp_fifo_num = 1;
int value[] = {0,0};
float y_0;


void pre_compute_tables() {
  // This function will precompute the cos and sin tables used in the rest of the program
  for(int precompute_counter = 0; precompute_counter < pre_compute_length; precompute_counter++){
    out_val_pre[precompute_counter] = (int) (cos(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter) * 2000.0 + 2050.0);
    i_mod_pre[precompute_counter] = (cos(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter));
    q_mod_pre[precompute_counter] = (-sin(twopi * CarrierFrequency * TimerInterruptInMicroSeconds * 1e-6 * precompute_counter));

  }
} //pre_compute_tables


void execute_BPF(int input_signal){
  bp_fifo_num++;
  
  
  if(bp_fifo_num >= bp_fir_length){
    bp_fifo_num = 0; 
  }
  x_0[bp_fifo_num] = input_signal;
  y_0 = x_0[bp_fifo_num] * bp_filter_coeff[0];
  
  for(int fir_counter = 1; fir_counter < bp_fir_length; fir_counter++){
    int fir_index = bp_fifo_num + fir_counter;
    if (fir_index >= bp_fir_length) {
      fir_index -= bp_fir_length;
    }
      y_0 += x_0[fir_index] * bp_filter_coeff[fir_counter];
  }

  current_i_mod = i_mod_pre[phase_counter];
  current_q_mod = q_mod_pre[phase_counter];
  
  //Serial.print("Input signal: ");
  //Serial.print(x_0[bp_fifo_num]);
  
 // Serial.print("bp_fifo_num: ");
 // Serial.print(bp_fifo_num);
  
  
  
  // Demodulation of the signal
  //x_0_i[lp_fifo_num] = (y_0 * current_i_mod);
  //x_0_q[lp_fifo_num] = (y_0 * current_q_mod);
}

void ISR() {
  // Calculation of loop for generating signal to output on DAC
  analogWrite(A14, out_val_pre[phase_counter]);
 
  
 
 //execute_BPF();
  
  phase_counter++;
  if (phase_counter >= pre_compute_length) phase_counter = 0;
  
} //ISR_repeat



void timer_setup() { //setup interrupts
  timer0.begin(ISR, TimerInterruptInMicroSeconds);  
} //timer_setup() 

void timer_stop() {
  timer0.end();
} //timer_stop() Teensy3


void setup() {
  //Analog input
  pinMode(A0, INPUT);
  
   analogWriteResolution(12); // Set up DAC resolution
   
   
  pre_compute_tables();  
  Serial.print("Hello! This is the DAQ and DSP program");
  analogWriteResolution(12); // Set up DAC resolution
  Serial2.begin(HW_BAUD_RATE);
  Serial2.flush();
  timer_setup();
  

}

int signal;
void loop() {
  signal = analogRead(A0); //Sample rate is 10kHz (Documentation online) 
  Serial.print("In: ");
  Serial.print(signal);
  execute_BPF(signal);
  Serial.print("Out: ");
  Serial.print(y_0);
  
}
