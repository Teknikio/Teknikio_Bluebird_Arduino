
/*
  This example is for continuous rotation servos connected to the
  Power/Motor shield. You can attach 2 servos.
   A continuous rotation servo can spin 360 degrees in either direction.
  The pulse tells the servo which direction to turn.
  To calibrate your servo set  myservo.write(90); and turn the screw on
  the bottom of the servo until it stops rotating
*/
#include <Servo.h>

Servo servoLeft;
Servo servoRight;

void setup()
{
  bluebird.begin();
  servoLeft.attach(13); //servo left
  servoRight.attach(15); //servo right
}

void loop() {
  //control Left Servo
  servoLeft.write(90);  // set servo to mid-point, this should stop the servo
  servoLeft.write(180); //turn to the right
  servoLeft.write(0); //turn to the left
  //control Right  Servo
  servoRight.write(90);  // set servo to mid-point, this should stop the servo
  servoRight.write(180); //turn to the right
  servoRight.write(0); //turn to the left
}
