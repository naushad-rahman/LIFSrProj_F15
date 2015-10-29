#include <AccelStepper.h>

int motorSpeed = 1200; //maximum steps per second (about 3rps / at 16 microsteps)
int motorAccel = 80000;

int motor_x_DirPin = 2; //digital pin 2
int motor_x_StepPin = 3; //digital 0pi0.......n 3
int motor_x_dir = -1; // reverse direction if need be
int x_pos = 0;

int motor_y_DirPin = 4; //digital pin 4
int motor_y_StepPin = 5; //digital pin 5
int motor_y_dir = -1; // reverse direction if need be
int y_pos = 0;

int motor_z_DirPin = 6; //digital pin 6
int motor_z_StepPin = 7; //digital pin 7
int motor_z_dir = -1; // reverse direction if need be
int z_pos = 0;

AccelStepper motor_x(1, motor_x_StepPin, motor_x_DirPin);
AccelStepper motor_y(1, motor_y_StepPin, motor_y_DirPin);
AccelStepper motor_z(1, motor_z_StepPin, motor_z_DirPin);


String incommingCommand;
String commandString;
int distance;

void processCommand(String in) {
  commandString = in.substring(0, 6);
  distance = in.substring(6).toInt();
  if (commandString == "1:x:+:") {        // X positive
    motor_x.move( distance * motor_x_dir);
x_pos += (motor_x_dir);
  } else if (commandString == "1:x:-:") { // X Negitive
    motor_x.move( distance * -1 * motor_x_dir);
x_pos -= distance;
  } else if (commandString == "1:y:+:") { // Y Positive
    motor_y.move( distance * motor_y_dir);
y_pos += distance;
  } else if (commandString == "1:y:-:") { // Y Negitive
    motor_y.move( distance * -1 * motor_y_dir);
y_pos -= distance;
  } else if (commandString == "1:z:+:") { // Z Positive
    motor_z.move( distance * motor_z_dir);
z_pos += distance;
  } else if (commandString == "1:z:-:") { // Z Negitive
    motor_z.move( distance * -1 * motor_z_dir);
z_pos -= distance;
  } else {
  }
}

void checkSerialCommand() {
  if (Serial.available() > 0) {
    // get incoming byte:
    int inByte = Serial.read();

    if ( (char)inByte == '\n' )
    {
      processCommand(incommingCommand);
      incommingCommand = "";
    } else {
      incommingCommand += (char)inByte;
    }
  }
}

void setup() {
  motor_x.setMaxSpeed(motorSpeed);
  motor_x.setSpeed(motorSpeed);
  motor_x.setAcceleration(motorAccel);
  //motor_x.moveTo(1000);

  motor_y.setMaxSpeed(motorSpeed);
  motor_y.setSpeed(motorSpeed);
  motor_y.setAcceleration(motorAccel);


  motor_z.setMaxSpeed(motorSpeed);
  motor_z.setSpeed(motorSpeed);
  motor_z.setAcceleration(motorAccel);

  Serial.begin(9600);
  Serial.print("begin\n");
}

void loop() {

  motor_x.run();
  motor_y.run();
  motor_z.run();

  checkSerialCommand();

}

