/*
  Send sequence of numbers (1 to 100) over serial port. Repeat indefinitely.
*/

// the setup routine runs once when you press reset:
int counter;
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  counter = 1;
}

// the loop routine runs over and over again forever:
void loop() {
  Serial.println(counter);
  counter++;
  if (counter==101) counter=1;
  delay(1000);        // delay in ms between reads for stability
}
