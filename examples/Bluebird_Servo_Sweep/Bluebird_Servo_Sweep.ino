/*
This example is for 180 degree  servos connected to the
Power/Motor shield. You can attach 2 servos using the 3 pin headers on the shield 
*/
#include <Bluebird_Teknikio.h>
#include <Servo.h> 
Servo servoLeft;
Servo servoRight;

int pos = 0;    // variable to store the servo position

void setup() 
{ 
  bluebird.begin();
  servoLeft.attach(13); //servo left
  servoRight.attach(15); //servo right
} 

void loop() {
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servoLeft.write(pos);              // tell servo to go to position in variable 'pos'
    servoRight.write(pos);             
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servoLeft.write(pos);              // tell servo to go to position in variable 'pos'
    servoRight.write(pos); 
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}
  
