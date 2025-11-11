#include "pitches.h"
#include <Servo.h>

#define BUZZER_PIN 8
#define SERVO_PIN 9
#define POT_PIN A0    //controlling melody speed 
#define DRUM_PIN A1   //controlling servo speed
#define BUTTON_PIN 2  //button for changing songs

Servo myservo;

//songs will be either 0, 1, or 2
int currentSong = 0;
bool buttonPressed = false;

//variables for melody 
int noteIndex = 0;
unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;
bool isPlaying = false;

//variables for servo
int servoPos = 0;
int servoStep = 1;
unsigned long lastServoUpdate = 0;
float servoDelay = 15;

//First song
int melody1[] = {
  NOTE_FS4, NOTE_FS4, NOTE_A4, NOTE_FS4, NOTE_A4,
  NOTE_FS4, NOTE_GS4, NOTE_GS4, NOTE_GS4, NOTE_A4, NOTE_B4,
  NOTE_E4, NOTE_GS4, NOTE_CS5, NOTE_GS4, NOTE_FS4
};
int durations1[] = {
  4, 4, 3, 4, 3, 4, 3, 8, 8, 4, 3, 4, 1, 4, 4, 1
};

//Second song
int melody2[] = {
  NOTE_GS4, NOTE_GS4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4,
  NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4,
  NOTE_DS5, NOTE_D5, NOTE_DS5, NOTE_C5, NOTE_DS5, NOTE_D5, NOTE_F5, NOTE_DS5
};
int durations2[] = {
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 2, 4, 2, 4, 2
};

//Third song 
int melody3[] = {
  NOTE_A4, REST, NOTE_B4, REST, NOTE_C5, REST, NOTE_A4, REST,
  NOTE_D5, REST, NOTE_E5, REST, NOTE_D5, REST
};
int durations3[] = {
  8, 8, 8, 8, 8, 8, 8, 4,
  8, 8, 8, 8, 2, 2
};

//function to return a pointer to an integer array 
int* getMelody() {
  switch (currentSong) {
    case 0: return melody1;
    case 1: return melody2;
    case 2: return melody3;
  }
  return melody1;
}

//function to return a pointer to the duration array
int* getDurations() {
  switch (currentSong) {
    case 0: return durations1;
    case 1: return durations2;
    case 2: return durations3;
  }
  return durations1;
}

//function to get the size of the song to know how many notes are in the current melody
int getSongSize() {
  switch (currentSong) {
    case 0: return sizeof(durations1) / sizeof(int);
    case 1: return sizeof(durations2) / sizeof(int);
    case 2: return sizeof(durations3) / sizeof(int);
  }
  return sizeof(durations1) / sizeof(int);
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  //button pressed mean LOW
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  myservo.attach(SERVO_PIN);
}

void loop() {
  //get the current time
  unsigned long currentTime = millis();
  //load the data of the current song
  int* melody = getMelody();
  int* durations = getDurations();
  int size = getSongSize();

  //read potentiometer
  int potValue = analogRead(POT_PIN);
  int drumValue = analogRead(DRUM_PIN);

  //melody
  int tempoFactor = map(potValue, 0, 1023, 500, 2500);

  //the speed of the servo sweep
  servoDelay = map(drumValue, 0, 1023, 1, 40);
  if (servoDelay > 39) servoDelay = 0;

  //switches songs if the button is pressed
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    //set the buttonPressed to true 
    buttonPressed = true;
    //choose the next song 
    currentSong = (currentSong + 1) % 3;
    //start playing the song from the beginning
    noteIndex = 0;
    isPlaying = false;
    noTone(BUZZER_PIN);
    delay(250);
  } else if (digitalRead(BUTTON_PIN) == HIGH) {
    //set buttonPressed to false
    buttonPressed = false;
  }

  //Use non-blocking to not affect the rest of the program
  if (currentTime - lastServoUpdate >= servoDelay) { //if servoDelay mills has passed
    lastServoUpdate = currentTime;
    //Change the angle of the servo by servoPos 
    myservo.write(servoPos);
    servoPos += servoStep;
    //Start decrementing if servo reaches 0 or 180 degrees
    if (servoPos >= 180 || servoPos <= 0) servoStep = -servoStep;
  }

  //Use non-blocking 
  //Start playing the next song if I am not currently playing a melody and there are still notes left
  if (!isPlaying && noteIndex < size) {
    //convert the note into milliseonds to get the duration of the melody
    int baseDuration = 1000 / durations[noteIndex];
    //control the speed of the melody based on the knob 
    noteDuration = (baseDuration * 1.30) * (1000.0 / tempoFactor); //smaller tempoFactor, faster melody plays 
    //play the melody on the buzzer pin for baseDuration 
    tone(BUZZER_PIN, melody[noteIndex], baseDuration);
    //save the current time to measure how long the note has been playing
    noteStartTime = currentTime;
    isPlaying = true;
  }

  //if the note is playing and the time since it started has reached the duration
  if (isPlaying && (currentTime - noteStartTime >= noteDuration)) {
    noTone(BUZZER_PIN);
    //move to the next note
    noteIndex++;
    isPlaying = false;
  }

  //replay the melody when it is finished 
  if (noteIndex >= size) {
    noteIndex = 0;
  }
}