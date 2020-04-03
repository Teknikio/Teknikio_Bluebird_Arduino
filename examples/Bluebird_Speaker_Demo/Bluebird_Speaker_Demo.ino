#include <Bluebird_Teknikio.h>
// Bluebird speaker demo

uint16_t measure = 0;
//fur elise, place any song here
int melody[] = {
  NOTE_E6, NOTE_DS6, NOTE_E6, NOTE_DS6,  NOTE_E6, NOTE_B5, NOTE_D6, NOTE_C6, NOTE_A5}; 

//temp for each note
int tempo[] = {
  12, 12, 12, 12, 12, 10, 12, 12, 6 }; 

//nimber of notes in song
uint8_t numberOfNotes = sizeof(melody)/sizeof(int);

void setup() {
  bluebird.begin();
  delay(50);
}
void loop() {
//plays notes set in melody array at assigned duration
 for (int n=0; n<numberOfNotes; n++) {
    int noteDuration = 1000 / tempo[n];
    bluebird.playTone(melody[n], noteDuration);
    delay(0.3*noteDuration);
  }
 
}
