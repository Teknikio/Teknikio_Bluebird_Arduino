
#include <Bluebird_Teknikio.h>


// the setup routine runs once when you press reset:
void setup() {
  bluebird.begin();
  pinMode(Pin1, INPUT);
  pinMode(Pin2, INPUT);
  pinMode(Pin3, INPUT);
  pinMode(Pin4, OUTPUT);
  delay (500);
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  //blink LED attached to Pin 4
  digitalWrite(Pin4, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(Pin4, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

  // print out the values of the inputs:
  Serial.println("Analog Read Pin1: ");
  Serial.println(analogRead(Pin1));
  Serial.println("Analog Read Pin2: ");
  Serial.println(analogRead(Pin2));
  
  delay(100);
  Serial.println("Digital Read Pin3: ");
  Serial.println(digitalRead(Pin3));
  delay(100);// delay in between reads for stability
}