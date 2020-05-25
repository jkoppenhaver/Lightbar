/*
 *  Description: Police Lightbar
 *  Filename:    Lightbar.ino
 *  Author:      J. Koppenhaver
 *  Libraries:   PololuLedStrip.h
 *    https://github.com/pololu/pololu-led-strip-arduino/releases/tag/1.2.0
 *    (Must use 1.2.0 to work with these strips.)
 *    The radio shack LED strips have 10 groups of 3 LEDs so if the strip
 *    being used has individually addressable LEDs some modification might
 *    be required.
 */


/*
 *  Light Pattern Descriptions
 *  1 - All red then all blue flashing.
 *  2 - Red blue every other switching.
 *  3 - One end red one end blue middle white.  White switches red and blue flash three times then switch.
 *  4 - Ends and middle blue and red with yellow moving across remaining segments.
 *  5 - Same as 4 but yellow moves opposite.
 *  6 - Red and blue ends sweeping back and forth with white switching middle.
 *  7 - Random red and blue.
 */

//If this is set to 1, debug information will be printed to the serial port.
#define DEBUG 1

#include <Arduino.h>
#include <PololuLedStrip.h>
#include <stdlib.h>


// Function Declarations
bool serialReadChar();
void parseSerialBuf();
void clearSerialBuf();
void serialFlush();
void initButtons();
int checkButtons();
void updateLights(byte);
void pattern0(byte);
void pattern1(byte);
void pattern2(byte);
void pattern3(byte);
void pattern4(byte);
void pattern5(byte);
void pattern6(byte);
void pattern7(byte);
void pattern21(byte);
void pattern22(byte);
void pattern24(byte);
void pattern25(byte);
void pattern27(byte);


// Create an ledStrip object on pin 12 and pin 11.
//These are the front and back lights
PololuLedStrip<9> ledStrip09;
PololuLedStrip<10> ledStrip10;
PololuLedStrip<11> ledStrip11;
PololuLedStrip<12> ledStrip12;

// Create a buffer for holding 10 colors.  Takes 30 bytes.
//One for each LED strip
#define LED_COUNT 10
rgb_color rear_buffer[LED_COUNT];
rgb_color front_buffer[LED_COUNT];

//Constant array to hold the pin numbers for the pattern select buttons
const byte buttonPins[] = {
  2,3,4,5,6,7,8,9,10};
const byte buttonPins_length = 9;
const byte frontRearSelect = 14;

//LED to show which LED strip is active.
const byte frontIndicator = 13;

//Variable to hold the current active light to edit.
byte activeEdit;

//Constants to hold the colors used in the patterns
//color = {R,B,G}
const rgb_color red = {
  255, 0, 0
};
const rgb_color blue = {
  0, 255, 0
};
const rgb_color white = {
  100, 150, 215
};
const rgb_color yellow = {
  255, 0, 255
};
const rgb_color off = {
  0, 0, 0
};

//Define is used to replace the words 'FRONT' and 'REAR' with 0 and 1 respectivly.
//This is only for code simplicity
#define FRONT 0
#define REAR 1

//This struct holds the variables for each process.
//When the main function switches processes it will store the old proccess in an enviroment and load the next state from another.
struct enviroment
{
  //This holds the number of the pattern currently running in that process
  byte currentPattern;
  //These holds upto 3 seperate sets of delay variables.
  unsigned int delays[3];
  unsigned int counters[3];
  unsigned long lastTimes[3];
  unsigned long lights[4][6];
  //These variables are used for pattern 7 only....
  int minDelay;
  int maxDelay;
};
//This array holds the environments for the two light pattern processes that normally run.
//One proccess for each side.
enviroment enviroments[2] = {
  {
    0
  }
  , {
    0
  }
};

//This method will run once at power on.
void setup()
{
  //Start up the serial port, for communication with the PC.
  Serial.begin(115200);
  //This method sets up the button pins.
  initButtons();
}
//These two variables tremporarily hold the pattern numbers for the front and back.
int front = 0;
int rear = 0;

bool serialReadingDone = false;
char serialBuf[20];
char *serialInPtr = serialBuf;

bool serialReadChar(){
  if(serialInPtr > &serialBuf[20]){
    clearSerialBuf();
  }
  *serialInPtr = Serial.read();
  if(*serialInPtr == '\n'){
    *serialInPtr = '\0';
    return true;
  }
  serialInPtr++;
  return false;
}

void parseSerialBuf(){
  char *rearStr = serialBuf;
  while(*rearStr != ':'  && *rearStr != '\0'){
    rearStr++;
  }
  if(*rearStr == ':'){
    *(rearStr++) = '\0';
  }
  else{
    rearStr = serialBuf;
  }
  front = atoi(serialBuf);
  rear = atoi(rearStr);
  clearSerialBuf();
  serialFlush();
}

void clearSerialBuf(){
  serialInPtr = serialBuf;
  for(int i = 0;i < 20;i++){
    serialBuf[i] = 0;
  }
  return;
}

//This method will loop for as long as the microprocessor is powered.
void loop()
{
  //Check to see if there is any data from the serial port waiting in the buffer.
  if (Serial.available())
  {
    if(serialReadChar()){
      parseSerialBuf();
    }
  }
  //Read in the values of the buttons.
  //int tmp = checkButtons();
  int tmp = -1;
  //If any of the buttons were pushed then change the lights.
  if(tmp >= 0){
    //Set the pattern read from the button to the currently active led strip.
    if(activeEdit == FRONT){
      front = tmp;
    }
    else{
      rear = tmp;
    }
  }
  //This switch case calls the pattern method for whichever pattern is currently on the front LEDs.
  //When a pattern method is called the main loop hands control of the proccesor to the pattern method so it can update the lights.
  //When the method is finished it will hand control of the processor back to the main loop method.
  switch (front)
  {
  case 0:
    pattern0(FRONT);
    break;
  case 1:
    pattern1(FRONT);
    break;
  case 2:
    pattern2(FRONT);
    break;
  case 3:
    pattern3(FRONT);
    break;
  case 4:
    pattern4(FRONT);
    break;
  case 5:
    pattern5(FRONT);
    break;
  case 6:
    pattern6(FRONT);
    break;
  case 7:
    pattern7(FRONT);
    break;
  case 21:
    pattern21(FRONT);
    break;
  case 22:
    pattern22(FRONT);
    break;
  case 24:
    pattern24(FRONT);
    break;
  case 25:
    pattern25(FRONT);
    break;
  case 27:
    pattern27(FRONT);
    break;
  }
  //This switch case calls the pattern method for whichever pattern is currently on the rear LEDs.
  //When a pattern method is called the main loop hands control of the proccesor to the pattern method so it can update the lights.
  //When the method is finished it will hand control of the processor back to the main loop method.
  switch (rear)
  {
  case 0:
    pattern0(REAR);
    break;
  case 1:
    pattern1(REAR);
    break;
  case 2:
    pattern2(REAR);
    break;
  case 3:
    pattern3(REAR);
    break;
  case 4:
    pattern4(REAR);
    break;
  case 5:
    pattern5(REAR);
    break;
  case 6:
    pattern6(REAR);
    break;
  case 7:
    pattern7(REAR);
    break;
  case 21:
    pattern21(REAR);
    break;
  case 22:
    pattern22(REAR);
    break;
  case 24:
    pattern24(REAR);
    break;
  case 25:
    pattern25(REAR);
    break;
  case 27:
    pattern27(REAR);
    break;
  }
  //This is the end of one loop, this method will continue to loop.
}

//This method clears any remaining data from the serial buffer.
//This method used to be included in the Serial class as Serial.flush() but since Arduino 1.0 it has different functionality.
//See http://www.arduino.cc/en/Serial/Flush for more information.
void serialFlush()
{
  while (Serial.available())
  {
    Serial.read();
  }
}

//This method configures the button pins and the activeEdit variable and the indicator.
//This method should only be called once.
void initButtons()
{
  for(int i = 0;i < buttonPins_length-1; i++){
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(frontRearSelect, INPUT_PULLUP);
  activeEdit = FRONT;
  pinMode(frontIndicator, OUTPUT);
  digitalWrite(frontIndicator, HIGH);
}

//This method reads the data from the buttons and performs three actions.
//First, it changes the activeEdit variable if that button has been pressed.
//Second, it  changes the front indicator to show the corect value.
//Third, it returns the pattern if a pattern button has been pressed or -1 if no button was pressed.
int checkButtons(){
  //Initiate a timer for debounce.
  //A static variable will only be initiated once so its value is kept between method calls.
  static unsigned long debounceTimer;
  //Check to see if the select button was pressed and to see if enough time has passed since the last press for debounce.
  if((digitalRead(frontRearSelect) == 0) && ((debounceTimer + 750) < millis())){
    //If it was pressed, toggle the activeEdit and set the front indicator accordingly.
    if(activeEdit == FRONT){
      activeEdit = REAR;
      digitalWrite(frontIndicator, LOW);
    }
    else{
      activeEdit = FRONT;
      digitalWrite(frontIndicator, HIGH);
    }
    //Software button debounce.
    debounceTimer = millis();
  }
  //Check each of the pattern buttons and if one has been pressed return that number.
  for(int i = 0;i < buttonPins_length-1; i++){
    if(digitalRead(buttonPins[i]) == 0){
      return i;
    }
  }
  //If none of the patter buttons have been pressed then return -1.
  return -1;
}

void updateLights(byte side){
  if(side == FRONT){
    ledStrip09.write(front_buffer, LED_COUNT);
    ledStrip10.write(front_buffer, LED_COUNT);
  }
  else{
    ledStrip11.write(rear_buffer, LED_COUNT);
    ledStrip12.write(rear_buffer, LED_COUNT);
  }
}

//This method turns all the LEDs off.
void pattern0(byte side)
{
  //Check what pattern that is currently running on the side that was passed in.
  //If the current pattern is different than this method's pattern the enviroment needs to be reset to the default values.
  if (enviroments[side].currentPattern != 0)
  {
    enviroments[side].currentPattern = 0;
    //This loop runs for each of the segments in the LED strip.
    for (byte i = 0; i < LED_COUNT; i++)
    {
      //Set the appropriate LED Strip segment to the off.
      if(side == FRONT){
        front_buffer[i] = off;
      }
      else{
        rear_buffer[i] = off;
      }
    }
    updateLights(side);
  }
}

//This method flashes all red and then all blue.
void pattern1(byte side)
{
  //Check what pattern that is currently running on the side that was passed in.
  //If the current pattern is different than this method's pattern the enviroment needs to be reset to the default values.
  if (enviroments[side].currentPattern != 1)
  {
    enviroments[side].currentPattern = 1;
    enviroments[side].delays[0] = 100;
    enviroments[side].counters[0] = 1;
    enviroments[side].lastTimes[0] = millis();
  }
  //Check if the delay has elapsed.
  //If the delay has not yet elapsed, hand control of the proccessor back to the main loop.
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    //If the counter is odd then set the LED array to red.
    //If it is even then set the array to blue.
    if ((enviroments[side].counters[0] % 2) == 1)
    {
      for (byte i = 0; i < LED_COUNT; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = red;
        }
        else
        {
          rear_buffer[i] = red;
        }
      }
    }
    else
    {
      for (byte i = 0; i < LED_COUNT; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = blue;
        }
        else
        {
          rear_buffer[i] = blue;
        }
      }
    }
    //Increment the counter and if it is greater than 2 roll it back to 1.
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 3)
    {
      enviroments[side].counters[0] = 1;
    }
    //Set the last run time to the current time.
    enviroments[side].lastTimes[0] = millis();
    updateLights(side);
  }
}

void pattern2(byte side)
{
  if (enviroments[side].currentPattern != 2)
  {
    enviroments[side].currentPattern = 2;
    enviroments[side].delays[0] = 100;
    enviroments[side].counters[0] = 1;
    enviroments[side].lastTimes[0] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    if ((enviroments[side].counters[0] % 2) == 1)
    {
      for (byte i = 0; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          front_buffer[i] = red;
        }
        else
        {
          rear_buffer[i] = red;
        }
      }
      for (byte i = 1; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          front_buffer[i] = blue;
        }
        else
        {
          rear_buffer[i] = blue;
        }
      }
    }
    else
    {
      for (byte i = 0; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          front_buffer[i] = blue;
        }
        else
        {
          rear_buffer[i] = blue;
        }
      }
      for (byte i = 1; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          front_buffer[i] = red;
        }
        else
        {
          rear_buffer[i] = red;
        }
      }
    }
    updateLights(side);
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 3)
    {
      enviroments[side].counters[0] = 1;
    }
    enviroments[side].lastTimes[0] = millis();
  }
}

void pattern3(byte side)
{
  if (enviroments[side].currentPattern != 3)
  {
    enviroments[side].currentPattern = 3;
    enviroments[side].delays[0] = 50;
    enviroments[side].delays[1] = 210;
    enviroments[side].counters[0] = 1;
    enviroments[side].counters[1] = 1;
    enviroments[side].lastTimes[0] = millis();
    enviroments[side].lastTimes[1] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    if (enviroments[side].counters[0] < 7)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        for (byte i = 0; i < 4; i++)
        {
          if (side == FRONT)
          {
            front_buffer[i] = blue;
          }
          else
          {
            rear_buffer[i] = blue;
          }
        }
      }
      else
      {
        for (byte i = 0; i < 4; i++)
        {
          if (side == FRONT)
          {
            front_buffer[i] = off;
          }
          else
          {
            rear_buffer[i] = off;
          }
        }
      }
      updateLights(side);
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        for (byte i = 6; i < 10; i++)
        {
          if (side == FRONT)
          {
            front_buffer[i] = red;
          }
          else
          {
            rear_buffer[i] = red;
          }
        }
      }
      else
      {
        for (byte i = 6; i < 10; i++)
        {
          if (side == FRONT)
          {
            front_buffer[i] = off;
          }
          else
          {
            rear_buffer[i] = off;
          }
        }
      }
      updateLights(side);
    }
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 13)
    {
      enviroments[side].counters[0] = 1;
    }
    enviroments[side].lastTimes[0] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[1] + enviroments[side].delays[1]))
  {
    if ((enviroments[side].counters[1] % 2) == 1)
    {
      if (side == FRONT)
      {
        front_buffer[4] = white;
        front_buffer[5] = off;
      }
      else
      {
        rear_buffer[4] = white;
        rear_buffer[5] = off;
      }
    }
    else
    {
      if (side == FRONT)
      {
        front_buffer[4] = off;
        front_buffer[5] = white;
      }
      else
      {
        rear_buffer[4] = off;
        rear_buffer[5] = white;
      }
    }
    updateLights(side);
    enviroments[side].counters[1]++;
    if (enviroments[side].counters[1] == 3)
    {
      enviroments[side].counters[1] = 1;
    }
    enviroments[side].lastTimes[1] = millis();
  }
}

void pattern4(byte side)
{
  if (enviroments[side].currentPattern != 4)
  {
    enviroments[side].currentPattern = 4;
    enviroments[side].delays[0] = 50;
    enviroments[side].delays[1] = 500;
    enviroments[side].counters[0] = 1;
    enviroments[side].counters[1] = 1;
    enviroments[side].lastTimes[0] = millis();
    enviroments[side].lastTimes[1] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    if (enviroments[side].counters[0] < 7)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          front_buffer[0] = blue;
          front_buffer[4] = blue;
        }
        else
        {
          rear_buffer[0] = blue;
          rear_buffer[4] = blue;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[0] = off;
          front_buffer[4] = off;
        }
        else
        {
          rear_buffer[0] = off;
          rear_buffer[4] = off;
        }
        front_buffer[0] = off;
        front_buffer[4] = off;
      }
      updateLights(side);
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          front_buffer[5] = red;
          front_buffer[9] = red;
        }
        else
        {
          rear_buffer[5] = red;
          rear_buffer[9] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[5] = off;
          front_buffer[9] = off;
        }
        else
        {
          rear_buffer[5] = off;
          rear_buffer[9] = off;
        }
      }
      updateLights(side);
    }
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 13)
    {
      enviroments[side].counters[0] = 1;
    }
    enviroments[side].lastTimes[0] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[1] + enviroments[side].delays[1]))
  {
    if (enviroments[side].counters[1] == 7)
    {
      for (byte i = 2; i < 4; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      for (byte i = 6; i < 9; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      updateLights(side);
      enviroments[side].counters[1] = 1;
    }
    if (enviroments[side].counters[1] < 4)
    {
      if (side == FRONT)
      {
        front_buffer[enviroments[side].counters[1]] = yellow;
      }
      else
      {
        rear_buffer[enviroments[side].counters[1]] = yellow;
      }
    }
    else
    {
      if (side == FRONT)
      {
        front_buffer[enviroments[side].counters[1] + 2] = yellow;
      }
      else
      {
        rear_buffer[enviroments[side].counters[1] + 2] = yellow;
      }
    }
    enviroments[side].counters[1]++;
    enviroments[side].lastTimes[1] = millis();
  }
}

void pattern5(byte side)
{
  if (enviroments[side].currentPattern != 5)
  {
    enviroments[side].currentPattern = 5;
    enviroments[side].delays[0] = 50;
    enviroments[side].delays[1] = 500;
    enviroments[side].counters[0] = 1;
    enviroments[side].counters[1] = 1;
    enviroments[side].lastTimes[0] = millis();
    enviroments[side].lastTimes[1] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    if (enviroments[side].counters[0] < 7)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          front_buffer[0] = blue;
          front_buffer[4] = blue;
        }
        else
        {
          rear_buffer[0] = blue;
          rear_buffer[4] = blue;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[0] = off;
          front_buffer[4] = off;
        }
        else
        {
          rear_buffer[0] = off;
          rear_buffer[4] = off;
        }
        front_buffer[0] = off;
        front_buffer[4] = off;
      }
      updateLights(side);
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          front_buffer[5] = red;
          front_buffer[9] = red;
        }
        else
        {
          rear_buffer[5] = red;
          rear_buffer[9] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[5] = off;
          front_buffer[9] = off;
        }
        else
        {
          rear_buffer[5] = off;
          rear_buffer[9] = off;
        }
      }
      updateLights(side);
    }
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 13)
    {
      enviroments[side].counters[0] = 1;
    }
    enviroments[side].lastTimes[0] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[1] + enviroments[side].delays[1]))
  {
    if (enviroments[side].counters[1] == 7)
    {
      for (byte i = 1; i < 4; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      for (byte i = 6; i < 8; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      updateLights(side);
      enviroments[side].counters[1] = 1;
    }
    if (enviroments[side].counters[1] < 4)
    {
      if (side == FRONT)
      {
        front_buffer[9 - enviroments[side].counters[1]] = yellow;
      }
      else
      {
        rear_buffer[9 - enviroments[side].counters[1]] = yellow;
      }
    }
    else
    {
      if (side == FRONT)
      {
        front_buffer[7 - enviroments[side].counters[1]] = yellow;
      }
      else
      {
        rear_buffer[7 - enviroments[side].counters[1]] = yellow;
      }
    }
    enviroments[side].counters[1]++;
    enviroments[side].lastTimes[1] = millis();
  }
}

void pattern6(byte side)
{
  if (enviroments[side].currentPattern != 6)
  {
    enviroments[side].currentPattern = 6;
    enviroments[side].delays[0] = 70;
    enviroments[side].delays[1] = 70;
    enviroments[side].delays[2] = 210;
    enviroments[side].counters[0] = 1;
    enviroments[side].counters[1] = 1;
    enviroments[side].counters[2] = 1;
    enviroments[side].lastTimes[0] = millis();
    enviroments[side].lastTimes[1] = millis();
    enviroments[side].lastTimes[2] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    if (enviroments[side].counters[0] < 5)
    {
      for (byte i = 0; i < 4; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      if (side == FRONT)
      {
        front_buffer[enviroments[side].counters[0] - 1] = blue;
      }
      else
      {
        rear_buffer[enviroments[side].counters[0] - 1] = blue;
      }
    }
    else
    {
      for (byte i = 0; i < 4; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      if (side == FRONT)
      {
        front_buffer[7 - enviroments[side].counters[0]] = blue;
      }
      else
      {
        rear_buffer[7 - enviroments[side].counters[0]] = blue;
      }
    }
    updateLights(side);
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 7)
    {
      enviroments[side].counters[0] = 1;
    }
    enviroments[side].lastTimes[0] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[1] + enviroments[side].delays[1]))
  {
    if (enviroments[side].counters[1] < 5)
    {
      for (byte i = 6; i < 10; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      if (side == FRONT)
      {
        front_buffer[10 - enviroments[side].counters[1]] = red;
      }
      else
      {
        rear_buffer[10 - enviroments[side].counters[1]] = red;
      }
    }
    else
    {
      for (byte i = 6; i < 10; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      if (side == FRONT)
      {
        front_buffer[enviroments[side].counters[1] + 2] = red;
      }
      else
      {
        rear_buffer[enviroments[side].counters[1] + 2] = red;
      }
    }
    updateLights(side);
    enviroments[side].counters[1]++;
    if (enviroments[side].counters[1] == 7)
    {
      enviroments[side].counters[1] = 1;
    }
    enviroments[side].lastTimes[1] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[2] + enviroments[side].delays[2]))
  {
    if ((enviroments[side].counters[2] % 2) == 1)
    {
      if (side == FRONT)
      {
        front_buffer[4] = white;
        front_buffer[5] = off;
      }
      else
      {
        rear_buffer[4] = white;
        rear_buffer[5] = off;
      }
    }
    else
    {
      if (side == FRONT)
      {
        front_buffer[4] = off;
        front_buffer[5] = white;
      }
      else
      {
        rear_buffer[4] = off;
        rear_buffer[5] = white;
      }
    }
    updateLights(side);
    enviroments[side].counters[2]++;
    if (enviroments[side].counters[2] == 3)
    {
      enviroments[side].counters[2] = 1;
    }
    enviroments[side].lastTimes[2] = millis();
  }
}

void pattern7(byte side)
{
  /*
            |Light1|Light2|Light3|....
   color    |0(red)|1(blu)|...........
   delay    |15(ms)|12(ms)|...........
   lastTime |32(ms)|98(ms)|..millis().
   pos      |  0   |  3   |...........
   */
  if (enviroments[side].currentPattern != 7)
  {
    enviroments[side].currentPattern = 7;
    enviroments[side].minDelay = 100;
    enviroments[side].maxDelay = 200;
    for (byte i = 0; i < 3; i++)
    {
      enviroments[side].lights[0][i] = 0;
    }
    for (byte i = 3; i < 6; i++)
    {
      enviroments[side].lights[0][i] = 1;
    }
    randomSeed(analogRead(0));
    for (byte i = 0; i < 6; i++)
    {
      enviroments[side].lights[1][i] = (unsigned long)random(enviroments[side].minDelay, enviroments[side].maxDelay + 1);
    }
    for (byte i = 0; i < 6; i++)
    {
      enviroments[side].lights[2][i] = millis();
    }
    for (byte i = 0; i < 6; i++)
    {
      unsigned long tmp;
      bool avail;
      do
      {
        tmp = (int)random(0, LED_COUNT);
        avail = true;
        for (byte j = 0; j < 6; j++)
        {
          if (enviroments[side].lights[3][j] == tmp)
          {
            avail = false;
            break;
          }
        }
      }
      while (!avail);
      enviroments[side].lights[3][i] = tmp;
    }
    for (byte i = 0; i < 6; i++)
    {
      if (enviroments[side].lights[0][i] == 0)
      {
        if (side == FRONT)
        {
          front_buffer[enviroments[side].lights[3][i]] = red;
        }
        else
        {
          rear_buffer[enviroments[side].lights[3][i]] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[enviroments[side].lights[3][i]] = blue;
        }
        else
        {
          rear_buffer[enviroments[side].lights[3][i]] = blue;
        }
      }
    }
    updateLights(side);
  }
  for (byte i = 0; i < 6; i++)
  {
    if (millis() > enviroments[side].lights[2][i] + enviroments[side].lights[1][i])
    {
      if (side == FRONT)
      {
        front_buffer[enviroments[side].lights[3][i]] = off;
      }
      else
      {
        rear_buffer[enviroments[side].lights[3][i]] = off;
      }
      unsigned long tmp;
      bool avail;
      do
      {
        tmp = (int)random(0, LED_COUNT);
        avail = true;
        for (byte j = 0; j < 6; j++)
        {
          if (enviroments[side].lights[3][j] == tmp)
          {
            avail = false;
            break;
          }
        }
      }
      while (!avail);
      enviroments[side].lights[3][i] = tmp;
      if (enviroments[side].lights[0][i] == 0)
      {
        if (side == FRONT)
        {
          front_buffer[enviroments[side].lights[3][i]] = red;
        }
        else
        {
          rear_buffer[enviroments[side].lights[3][i]] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[enviroments[side].lights[3][i]] = blue;
        }
        else
        {
          rear_buffer[enviroments[side].lights[3][i]] = blue;
        }
      }
      enviroments[side].lights[1][i] = (int)random(enviroments[side].minDelay, enviroments[side].maxDelay + 1);
      enviroments[side].lights[2][i] = millis();
      updateLights(side);
    }
  }
}


//This method flashes all red and then all white.
void pattern21(byte side)
{
  //Check what pattern that is currently running on the side that was passed in.
  //If the current pattern is different than this method's pattern the enviroment needs to be reset to the default values.
  if (enviroments[side].currentPattern != 1)
  {
    enviroments[side].currentPattern = 1;
    enviroments[side].delays[0] = 100;
    enviroments[side].counters[0] = 1;
    enviroments[side].lastTimes[0] = millis();
  }
  //Check if the delay has elapsed.
  //If the delay has not yet elapsed, hand control of the proccessor back to the main loop.
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    //If the counter is odd then set the LED array to red.
    //If it is even then set the array to blue.
    if ((enviroments[side].counters[0] % 2) == 1)
    {
      for (byte i = 0; i < LED_COUNT; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = red;
        }
        else
        {
          rear_buffer[i] = red;
        }
      }
    }
    else
    {
      for (byte i = 0; i < LED_COUNT; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = white;
        }
        else
        {
          rear_buffer[i] = white;
        }
      }
    }
    //Write the appropriate array to the appropriate LED strip.
    updateLights(side);
    //Increment the counter and if it is greater than 2 roll it back to 1.
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 3)
    {
      enviroments[side].counters[0] = 1;
    }
    //Set the last run time to the current time.
    enviroments[side].lastTimes[0] = millis();
  }
}

void pattern22(byte side)
{
  if (enviroments[side].currentPattern != 2)
  {
    enviroments[side].currentPattern = 2;
    enviroments[side].delays[0] = 100;
    enviroments[side].counters[0] = 1;
    enviroments[side].lastTimes[0] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    if ((enviroments[side].counters[0] % 2) == 1)
    {
      for (byte i = 0; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          front_buffer[i] = red;
        }
        else
        {
          rear_buffer[i] = red;
        }
      }
      for (byte i = 1; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          front_buffer[i] = white;
        }
        else
        {
          rear_buffer[i] = white;
        }
      }
    }
    else
    {
      for (byte i = 0; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          front_buffer[i] = white;
        }
        else
        {
          rear_buffer[i] = white;
        }
      }
      for (byte i = 1; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          front_buffer[i] = red;
        }
        else
        {
          rear_buffer[i] = red;
        }
      }
    }
    updateLights(side);

    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 3)
    {
      enviroments[side].counters[0] = 1;
    }
    enviroments[side].lastTimes[0] = millis();
  }
}

void pattern24(byte side)
{
  if (enviroments[side].currentPattern != 4)
  {
    enviroments[side].currentPattern = 4;
    enviroments[side].delays[0] = 50;
    enviroments[side].delays[1] = 500;
    enviroments[side].counters[0] = 1;
    enviroments[side].counters[1] = 1;
    enviroments[side].lastTimes[0] = millis();
    enviroments[side].lastTimes[1] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    if (enviroments[side].counters[0] < 7)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          front_buffer[0] = white;
          front_buffer[4] = white;
        }
        else
        {
          rear_buffer[0] = white;
          rear_buffer[4] = white;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[0] = off;
          front_buffer[4] = off;
        }
        else
        {
          rear_buffer[0] = off;
          rear_buffer[4] = off;
        }
        front_buffer[0] = off;
        front_buffer[4] = off;
      }
      updateLights(side);
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          front_buffer[5] = red;
          front_buffer[9] = red;
        }
        else
        {
          rear_buffer[5] = red;
          rear_buffer[9] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[5] = off;
          front_buffer[9] = off;
        }
        else
        {
          rear_buffer[5] = off;
          rear_buffer[9] = off;
        }
      }
      updateLights(side);
    }
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 13)
    {
      enviroments[side].counters[0] = 1;
    }
    enviroments[side].lastTimes[0] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[1] + enviroments[side].delays[1]))
  {
    if (enviroments[side].counters[1] == 7)
    {
      for (byte i = 2; i < 4; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      for (byte i = 6; i < 9; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      updateLights(side);
      enviroments[side].counters[1] = 1;
    }
    if (enviroments[side].counters[1] < 4)
    {
      if (side == FRONT)
      {
        front_buffer[enviroments[side].counters[1]] = yellow;
      }
      else
      {
        rear_buffer[enviroments[side].counters[1]] = yellow;
      }
    }
    else
    {
      if (side == FRONT)
      {
        front_buffer[enviroments[side].counters[1] + 2] = yellow;
      }
      else
      {
        rear_buffer[enviroments[side].counters[1] + 2] = yellow;
      }
    }
    enviroments[side].counters[1]++;
    enviroments[side].lastTimes[1] = millis();
  }
}

void pattern25(byte side)
{
  if (enviroments[side].currentPattern != 5)
  {
    enviroments[side].currentPattern = 5;
    enviroments[side].delays[0] = 50;
    enviroments[side].delays[1] = 500;
    enviroments[side].counters[0] = 1;
    enviroments[side].counters[1] = 1;
    enviroments[side].lastTimes[0] = millis();
    enviroments[side].lastTimes[1] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[0] + enviroments[side].delays[0]))
  {
    if (enviroments[side].counters[0] < 7)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          front_buffer[0] = white;
          front_buffer[4] = white;
        }
        else
        {
          rear_buffer[0] = white;
          rear_buffer[4] = white;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[0] = off;
          front_buffer[4] = off;
        }
        else
        {
          rear_buffer[0] = off;
          rear_buffer[4] = off;
        }
        front_buffer[0] = off;
        front_buffer[4] = off;
      }
      updateLights(side);
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          front_buffer[5] = red;
          front_buffer[9] = red;
        }
        else
        {
          rear_buffer[5] = red;
          rear_buffer[9] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[5] = off;
          front_buffer[9] = off;
        }
        else
        {
          rear_buffer[5] = off;
          rear_buffer[9] = off;
        }
      }
      updateLights(side);
    }
    enviroments[side].counters[0]++;
    if (enviroments[side].counters[0] == 13)
    {
      enviroments[side].counters[0] = 1;
    }
    enviroments[side].lastTimes[0] = millis();
  }
  if (millis() > (enviroments[side].lastTimes[1] + enviroments[side].delays[1]))
  {
    if (enviroments[side].counters[1] == 7)
    {
      for (byte i = 1; i < 4; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      for (byte i = 6; i < 8; i++)
      {
        if (side == FRONT)
        {
          front_buffer[i] = off;
        }
        else
        {
          rear_buffer[i] = off;
        }
      }
      updateLights(side);
      enviroments[side].counters[1] = 1;
    }
    if (enviroments[side].counters[1] < 4)
    {
      if (side == FRONT)
      {
        front_buffer[9 - enviroments[side].counters[1]] = yellow;
      }
      else
      {
        rear_buffer[9 - enviroments[side].counters[1]] = yellow;
      }
    }
    else
    {
      if (side == FRONT)
      {
        front_buffer[7 - enviroments[side].counters[1]] = yellow;
      }
      else
      {
        rear_buffer[7 - enviroments[side].counters[1]] = yellow;
      }
    }
    enviroments[side].counters[1]++;
    enviroments[side].lastTimes[1] = millis();
  }
}

void pattern27(byte side)
{
  /*
            |Light1|Light2|Light3|....
   color    |0(red)|1(blu)|...........
   delay    |15(ms)|12(ms)|...........
   lastTime |32(ms)|98(ms)|..millis().
   pos      |  0   |  3   |...........
   */
  if (enviroments[side].currentPattern != 7)
  {
    enviroments[side].currentPattern = 7;
    enviroments[side].minDelay = 100;
    enviroments[side].maxDelay = 200;
    for (byte i = 0; i < 3; i++)
    {
      enviroments[side].lights[0][i] = 0;
    }
    for (byte i = 3; i < 6; i++)
    {
      enviroments[side].lights[0][i] = 1;
    }
    randomSeed(analogRead(0));
    for (byte i = 0; i < 6; i++)
    {
      enviroments[side].lights[1][i] = (int)random(enviroments[side].minDelay, enviroments[side].maxDelay + 1);
    }
    for (byte i = 0; i < 6; i++)
    {
      enviroments[side].lights[2][i] = millis();
    }
    for (byte i = 0; i < 6; i++)
    {
      unsigned long tmp;
      bool avail;
      do
      {
        tmp = (int)random(0, LED_COUNT);
        avail = true;
        for (byte j = 0; j < 6; j++)
        {
          if (enviroments[side].lights[3][j] == tmp)
          {
            avail = false;
            break;
          }
        }
      }
      while (!avail);
      enviroments[side].lights[3][i] = tmp;
    }
    for (byte i = 0; i < 6; i++)
    {
      if (enviroments[side].lights[0][i] == 0)
      {
        if (side == FRONT)
        {
          front_buffer[enviroments[side].lights[3][i]] = red;
        }
        else
        {
          rear_buffer[enviroments[side].lights[3][i]] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[enviroments[side].lights[3][i]] = white;
        }
        else
        {
          rear_buffer[enviroments[side].lights[3][i]] = white;
        }
      }
    }
    updateLights(side);
  }
  for (byte i = 0; i < 6; i++)
  {
    if (millis() > enviroments[side].lights[2][i] + enviroments[side].lights[1][i])
    {
      if (side == FRONT)
      {
        front_buffer[enviroments[side].lights[3][i]] = off;
      }
      else
      {
        rear_buffer[enviroments[side].lights[3][i]] = off;
      }
      unsigned long tmp;
      bool avail;
      do
      {
        tmp = (int)random(0, LED_COUNT);
        avail = true;
        for (byte j = 0; j < 6; j++)
        {
          if (enviroments[side].lights[3][j] == tmp)
          {
            avail = false;
            break;
          }
        }
      }
      while (!avail);
      enviroments[side].lights[3][i] = tmp;
      if (enviroments[side].lights[0][i] == 0)
      {
        if (side == FRONT)
        {
          front_buffer[enviroments[side].lights[3][i]] = red;
        }
        else
        {
          rear_buffer[enviroments[side].lights[3][i]] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          front_buffer[enviroments[side].lights[3][i]] = white;
        }
        else
        {
          rear_buffer[enviroments[side].lights[3][i]] = white;
        }
      }
      enviroments[side].lights[1][i] = (int)random(enviroments[side].minDelay, enviroments[side].maxDelay + 1);
      enviroments[side].lights[2][i] = millis();
      updateLights(side);
    }
  }
}