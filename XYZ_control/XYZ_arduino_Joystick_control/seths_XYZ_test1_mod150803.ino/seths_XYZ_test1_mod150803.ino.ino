
//////////////////////////////////////////////////////////////////
//Using the easy stepper with your arduino
//use rotate and/or rotateDeg to controll stepper motor
//speed is any number from .01 -> 1 with 1 being fastest -
//Slower Speed == Stronger movement
/////////////////////////////////////////////////////////////////
const int VERT = 5; // analog
const int HORIZ = 4; // analog
const int SEL = 0; // digital

volatile int steps_xy = 200;
volatile int steps_y_adjusted = 37;
volatile int steps_z = 100;

#define FAST 0.8
#define SLOW 0.1

#define DIR_PIN_X 2
#define STEP_PIN_X 3
#define DIR_PIN_Y 5
#define STEP_PIN_Y 4
#define DIR_PIN_Z 6
#define STEP_PIN_Z 7

void setup() {
  // make the SEL line an input
  pinMode(SEL,INPUT);
  // turn on the pull-up resistor for the SEL line (see http://arduino.cc/en/Tutorial/DigitalPins)
  digitalWrite(SEL,HIGH);
  // set up serial port for output
  Serial.begin(9600);
  pinMode(DIR_PIN_X, OUTPUT);
  pinMode(STEP_PIN_X, OUTPUT);
  pinMode(DIR_PIN_Y, OUTPUT);
  pinMode(STEP_PIN_Y, OUTPUT);
  pinMode(DIR_PIN_Z, OUTPUT);
  pinMode(STEP_PIN_Z, OUTPUT);
}

int z_active =0;
void loop(){
  int vertical, horizontal, stick_not_pressed;
  vertical = analogRead(VERT); // will be 0-1023
  horizontal = analogRead(HORIZ); // will be 0-1023
  stick_not_pressed = digitalRead(SEL); // will be HIGH (1) if not pressed, and LOW (0) if pressed
  if(!stick_not_pressed){
    delay(600); // 0.6 second to debounce
    z_active = !z_active;
    //Serial.println("Debounced Z");
  }
  // if(z_active==1){
  // if(vertical > 750){
  // Serial.println("+Z");
  // rotateDeg(30, .4, DIR_PIN_Z, STEP_PIN_Z);
  // } else if(vertical < 250){
  // Serial.println("-Z");
  // rotateDeg(-30, .4, DIR_PIN_Z, STEP_PIN_Z); //reverse
  // } else{
  // }
  // }else{
  // if(vertical > 750){
  // rotateDeg(30, .4, DIR_PIN_X, STEP_PIN_X);
  // Serial.println("+X");
  // } else if(vertical < 250){
  // rotateDeg(-30, .4, DIR_PIN_X, STEP_PIN_X); //reverse
  // Serial.println("-X");
  // } else if(horizontal > 750){
  // rotateDeg(30, .4, DIR_PIN_Y, STEP_PIN_Y);
  // Serial.println("+Y");
  // } else if(horizontal < 250){
  // Serial.println("-Y");
  // rotateDeg(-30, .4, DIR_PIN_Y, STEP_PIN_Y);
  // } else {
  // }
  // }
  //Serial.print(z_active); Serial.print(", "); Serial.print(vertical); Serial.print(", ");
  //Serial.println( horizontal );
  if(z_active==1){
    if(vertical > 850){
      rotate(steps_z, FAST, DIR_PIN_Z, STEP_PIN_Z);
      rotate(-steps_y_adjusted, FAST, DIR_PIN_Y, STEP_PIN_Y);
      Serial.println("Down");
    } 
    else if(vertical > 550 && vertical < 850){
      rotate(steps_z, SLOW, DIR_PIN_Z, STEP_PIN_Z);
      rotate(-steps_y_adjusted, SLOW, DIR_PIN_Y, STEP_PIN_Y);
      Serial.println("Down");
    } 
    else if(vertical > 150 && vertical < 450){
      rotate(-steps_z, SLOW, DIR_PIN_Z, STEP_PIN_Z);
      rotate(steps_y_adjusted, SLOW, DIR_PIN_Y, STEP_PIN_Y);
      Serial.println("Up");
    } 
    else if(vertical < 150){
      rotate(-steps_z, FAST, DIR_PIN_Z, STEP_PIN_Z); //reverse
      rotate(steps_y_adjusted, FAST, DIR_PIN_Y, STEP_PIN_Y);
      Serial.println("Up");
    }
    else if(horizontal > 850)
    {
      steps_xy = 200;
      steps_y_adjusted = 37;
      steps_z = 100;
      Serial.println("Fast Mode");

    }
    else if(horizontal <150)
    {
      steps_xy = 20;
      steps_y_adjusted = 4;
      steps_z = 10;
      Serial.println("Slow Mode");
    }
  }
  else{
    if(vertical > 850){
      rotate(steps_xy, FAST, DIR_PIN_X, STEP_PIN_X);
      Serial.println("Left");
    } 
    else if(vertical > 650 && vertical < 850){
      rotate(steps_xy, SLOW, DIR_PIN_X, STEP_PIN_X);
      Serial.println("Left");
    } 
    else if(vertical > 150 && vertical < 450){
      rotate(-steps_xy, SLOW, DIR_PIN_X, STEP_PIN_X); //reverse
      Serial.println("Right");
    } 
    else if(vertical < 150){
      rotate(-steps_xy, FAST, DIR_PIN_X, STEP_PIN_X); //reverse
      Serial.println("Right");
      
    } 
    else if(horizontal > 850){
      rotate(steps_xy, FAST, DIR_PIN_Y, STEP_PIN_Y);
      Serial.println("Back");
    } 
    else if(horizontal > 650 && horizontal < 850){
        rotate(steps_xy, SLOW, DIR_PIN_Y, STEP_PIN_Y);
        Serial.println("Back");
    } 
    else if(horizontal > 150 && horizontal < 450){
        rotate(-steps_xy, SLOW, DIR_PIN_Y, STEP_PIN_Y);
        Serial.println("Forward");
    } 
    else if(horizontal < 150){
        rotate(-steps_xy, FAST, DIR_PIN_Y, STEP_PIN_Y);
        Serial.println("Forward");
    }
  }
}

void rotateDeg(float deg, float speed, int dir_p, int step_p){
  //rotate a specific number of degrees (negitive for reverse movement)
  //speed is any number from .01 -> 1 with 1 being fastest - Slower is stronger
  int dir = (deg > 0)? HIGH:LOW;
  digitalWrite(dir_p,dir);
  int steps = abs(deg)*(1/0.225);
  float usDelay = (1/speed) * 70;
  for(int i=0; i < steps; i++){
    digitalWrite(step_p, HIGH);
    delayMicroseconds(usDelay);
    digitalWrite(step_p, LOW);
    delayMicroseconds(usDelay);
  }
}

void rotate(int steps, float speed, int dir_p, int step_p){
  //rotate a specific number of microsteps (8 microsteps per step) - (negitive for reverse movement)
  //speed is any number from .01 -> 1 with 1 being fastest - Slower is stronger
  int dir = (steps > 0)? HIGH:LOW;
  steps = abs(steps);
  digitalWrite(dir_p,dir);
  float usDelay = (1/speed) * 70;
  for(int i=0; i < steps; i++){
    digitalWrite(step_p, HIGH);
    delayMicroseconds(usDelay);
    digitalWrite(step_p, LOW);
    delayMicroseconds(usDelay);
  }
}
