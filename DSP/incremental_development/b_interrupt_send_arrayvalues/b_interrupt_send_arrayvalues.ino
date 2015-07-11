/*
  Sequentially send values from an array over the serial port. Repeat indefinitely. 
  Use an interrupt timer to assign the value to be written in the main loop.
*/

#include <stdio.h>

#define LENGTH_OF_TEST_SIGNAL 10

float test_signal[LENGTH_OF_TEST_SIGNAL] = {
        1.        ,  0.80901699,  0.30901699, -0.30901699, -0.80901699,
       -1.        , -0.80901699, -0.30901699,  0.30901699,  0.80901699};

int timer_interval_us = 1000;
int volatile counter;
float volatile value;
boolean volatile value_ready;

IntervalTimer timer0;

void timer_callback(void) {
    value = test_signal[counter];
    counter++;
    if (counter == LENGTH_OF_TEST_SIGNAL) counter = 0;
}

void setup() {
    // initialize serial communication at 9600 bits per second:
    Serial.begin(9600);
    Serial.flush();
    counter = 0;
    value_ready = false;
    timer0.begin(timer_callback, timer_interval_us);
}

void loop() {
    if (value_ready) {
        int temp = int (1000*value);
        //Serial.print(counter);
        Serial.println( value );
        value_ready = false;
    }
}

