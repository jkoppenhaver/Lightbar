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


// Function Declarations
void serialFlush();
void initButtons();
int checkButtons();
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
PololuLedStrip<12> ledStrip12;
PololuLedStrip<11> ledStrip11;

// Create a buffer for holding 10 colors.  Takes 30 bytes.
//One for each LED strip
#define LED_COUNT 10
rgb_color colors12[LED_COUNT];
rgb_color colors11[LED_COUNT];

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
  100, 150, 255
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
  int delays[3];
  int counters[3];
  long lastTimes[3];
  long lights[4][6];
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

//This method will loop for as long as the microprocessor is powered.
void loop()
{
  //Check to see if there is any data from the serial port waiting in the buffer.
  if (Serial.available())
  {
    //If there is data it needs to be handled.
    //Read in the first character.
    char next = Serial.read();
    //If debug is enabled print some information to the serial port.
    if(DEBUG){
      Serial.print("DEBUG:");
      Serial.print("First number read - ");
      Serial.println(next);
    }
    //If the first character is a number 0-9 then turn off the front light strip and write the character to the front variable.
    if ((next >= '0') && (next <= '9'))
    {
      pattern0(FRONT);
      //This converts the ascii character to an int.
      //If this is confusing then look at the characters and integer values on an ascii table.
      front = next - 48;
    }
    //If the next character is not a ':' and there is still more data then throw out the current character and read in the next one.
    while ((next != ':') && (Serial.available()))
    {
      next = Serial.read();
    }
    //Throw out the colon and read in the next character.
    next = Serial.read();
    //If debug is enabled print some information to the serial port.
    if(DEBUG){
      Serial.print("DEBUG:");
      Serial.print("Second number read - ");
      Serial.println(next);
      Serial.print("DEBUG:");
      Serial.print("Second number present - ");
      Serial.println((next >= '0') && (next <= '9'));
    }
    //If the character after the ':' is a number 0-9 then turn off the rear light strip and write the character to the rear variable.
    if ((next >= '0') && (next <= '9'))
    {
      pattern0(REAR);
      //This converts the ascii character to an int.
      //If this is confusing then look at the characters and integer values on an ascii table.
      rear = next - 48;
    }
    //Clear any remaining characters from the serial buffer.
    serialFlush();
    //Print out the front and rear patterns to the serial port.
    Serial.print("Front lights set to pattern ");
    Serial.print(front);
    Serial.println(".");
    Serial.print("Rear lights set to pattern ");
    Serial.print(rear);
    Serial.println(".");
  }
  //Read in the values of the buttons.
  //int tmp = checkButtons();
  int tmp = -1;
  //If any of the buttons were pushed then change the lights.
  if(tmp >= 0){
    if(DEBUG){
      Serial.print("DEBUG:");
      Serial.print("Button read - ");
      Serial.println(tmp);
      Serial.print("DEBUG:");
      Serial.print("Current Active Side - ");
      Serial.println(activeEdit);
    }
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
  static long debounceTimer;
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

//This method turns all the LEDs off.
void pattern0(byte side)
{
  //This loop runs for each of the segments in the LED strip.
  for (byte i = 0; i < LED_COUNT; i++)
  {
    //Set the appropriate LED Strip segment to the off.
    if(side == FRONT){
      colors11[i] = off;
    }
    else{
      colors12[i] = off;
    }
  }
  //Write the array data to the LED strips.
  if(side == FRONT){
    ledStrip11.write(colors11, LED_COUNT);
  }
  else{
    ledStrip12.write(colors12, LED_COUNT);
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
          colors11[i] = red;
        }
        else
        {
          colors12[i] = red;
        }
      }
    }
    else
    {
      for (byte i = 0; i < LED_COUNT; i++)
      {
        if (side == FRONT)
        {
          colors11[i] = blue;
        }
        else
        {
          colors12[i] = blue;
        }
      }
    }
    //Write the appropriate array to the appropriate LED strip.
    if (side == FRONT)
    {
      ledStrip11.write(colors11, LED_COUNT);
    }
    else
    {
      ledStrip12.write(colors12, LED_COUNT);
    }
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
          colors11[i] = red;
        }
        else
        {
          colors12[i] = red;
        }
      }
      for (byte i = 1; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          colors11[i] = blue;
        }
        else
        {
          colors12[i] = blue;
        }
      }
    }
    else
    {
      for (byte i = 0; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          colors11[i] = blue;
        }
        else
        {
          colors12[i] = blue;
        }
      }
      for (byte i = 1; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          colors11[i] = red;
        }
        else
        {
          colors12[i] = red;
        }
      }
    }
    if (side == FRONT)
    {
      ledStrip11.write(colors11, LED_COUNT);
    }
    else
    {
      ledStrip12.write(colors12, LED_COUNT);
    }

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
            colors11[i] = blue;
            ledStrip11.write(colors11, LED_COUNT);
          }
          else
          {
            colors12[i] = blue;
            ledStrip12.write(colors12, LED_COUNT);
          }
        }
      }
      else
      {
        for (byte i = 0; i < 4; i++)
        {
          if (side == FRONT)
          {
            colors11[i] = off;
            ledStrip11.write(colors11, LED_COUNT);
          }
          else
          {
            colors12[i] = off;
            ledStrip12.write(colors12, LED_COUNT);
          }
        }
      }
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        for (byte i = 6; i < 10; i++)
        {
          if (side == FRONT)
          {
            colors11[i] = red;
            ledStrip11.write(colors11, LED_COUNT);
          }
          else
          {
            colors12[i] = red;
            ledStrip12.write(colors12, LED_COUNT);
          }
        }
      }
      else
      {
        for (byte i = 6; i < 10; i++)
        {
          if (side == FRONT)
          {
            colors11[i] = off;
            ledStrip11.write(colors11, LED_COUNT);
          }
          else
          {
            colors12[i] = off;
            ledStrip12.write(colors12, LED_COUNT);
          }
        }
      }
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
        colors11[4] = white;
        colors11[5] = off;
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        colors12[4] = white;
        colors12[5] = off;
        ledStrip12.write(colors12, LED_COUNT);
      }
    }
    else
    {
      if (side == FRONT)
      {
        colors11[4] = off;
        colors11[5] = white;
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        colors12[4] = off;
        colors12[5] = white;
        ledStrip12.write(colors12, LED_COUNT);
      }
    }
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
          colors11[0] = blue;
          colors11[4] = blue;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[0] = blue;
          colors12[4] = blue;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[0] = off;
          colors11[4] = off;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[0] = off;
          colors12[4] = off;
          ledStrip12.write(colors12, LED_COUNT);
        }
        colors11[0] = off;
        colors11[4] = off;
        ledStrip11.write(colors11, LED_COUNT);
      }
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          colors11[5] = red;
          colors11[9] = red;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[5] = red;
          colors12[9] = red;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[5] = off;
          colors11[9] = off;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[5] = off;
          colors12[9] = off;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
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
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      for (byte i = 6; i < 9; i++)
      {
        if (side == FRONT)
        {
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      if (side == FRONT)
      {
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        ledStrip12.write(colors12, LED_COUNT);
      }
      enviroments[side].counters[1] = 1;
    }
    if (enviroments[side].counters[1] < 4)
    {
      if (side == FRONT)
      {
        colors11[enviroments[side].counters[1]] = yellow;
      }
      else
      {
        colors12[enviroments[side].counters[1]] = yellow;
      }
    }
    else
    {
      if (side == FRONT)
      {
        colors11[enviroments[side].counters[1] + 2] = yellow;
      }
      else
      {
        colors12[enviroments[side].counters[1] + 2] = yellow;
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
          colors11[0] = blue;
          colors11[4] = blue;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[0] = blue;
          colors12[4] = blue;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[0] = off;
          colors11[4] = off;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[0] = off;
          colors12[4] = off;
          ledStrip12.write(colors12, LED_COUNT);
        }
        colors11[0] = off;
        colors11[4] = off;
        ledStrip11.write(colors11, LED_COUNT);
      }
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          colors11[5] = red;
          colors11[9] = red;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[5] = red;
          colors12[9] = red;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[5] = off;
          colors11[9] = off;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[5] = off;
          colors12[9] = off;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
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
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      for (byte i = 6; i < 8; i++)
      {
        if (side == FRONT)
        {
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      if (side == FRONT)
      {
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        ledStrip12.write(colors12, LED_COUNT);
      }
      enviroments[side].counters[1] = 1;
    }
    if (enviroments[side].counters[1] < 4)
    {
      if (side == FRONT)
      {
        colors11[9 - enviroments[side].counters[1]] = yellow;
      }
      else
      {
        colors12[9 - enviroments[side].counters[1]] = yellow;
      }
    }
    else
    {
      if (side == FRONT)
      {
        colors11[7 - enviroments[side].counters[1]] = yellow;
      }
      else
      {
        colors12[7 - enviroments[side].counters[1]] = yellow;
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
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      if (side == FRONT)
      {
        colors11[enviroments[side].counters[0] - 1] = blue;
      }
      else
      {
        colors12[enviroments[side].counters[0] - 1] = blue;
      }
    }
    else
    {
      for (byte i = 0; i < 4; i++)
      {
        if (side == FRONT)
        {
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      if (side == FRONT)
      {
        colors11[7 - enviroments[side].counters[0]] = blue;
      }
      else
      {
        colors12[7 - enviroments[side].counters[0]] = blue;
      }
    }
    if (side == FRONT)
    {
      ledStrip11.write(colors11, LED_COUNT);
    }
    else
    {
      ledStrip12.write(colors12, LED_COUNT);
    }
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
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      if (side == FRONT)
      {
        colors11[10 - enviroments[side].counters[1]] = red;
      }
      else
      {
        colors12[10 - enviroments[side].counters[1]] = red;
      }
    }
    else
    {
      for (byte i = 6; i < 10; i++)
      {
        if (side == FRONT)
        {
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      if (side == FRONT)
      {
        colors11[enviroments[side].counters[1] + 2] = red;
      }
      else
      {
        colors12[enviroments[side].counters[1] + 2] = red;
      }
    }
    if (side == FRONT)
    {
      ledStrip11.write(colors11, LED_COUNT);
    }
    else
    {
      ledStrip12.write(colors12, LED_COUNT);
    }
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
        colors11[4] = white;
        colors11[5] = off;
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        colors12[4] = white;
        colors12[5] = off;
        ledStrip12.write(colors12, LED_COUNT);
      }
    }
    else
    {
      if (side == FRONT)
      {
        colors11[4] = off;
        colors11[5] = white;
        ledStrip11.write(colors12, LED_COUNT);
      }
      else
      {
        colors12[4] = off;
        colors12[5] = white;
        ledStrip12.write(colors11, LED_COUNT);
      }
    }
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
      enviroments[side].lights[1][i] = (int)random(enviroments[side].minDelay, enviroments[side].maxDelay + 1);
    }
    for (byte i = 0; i < 6; i++)
    {
      enviroments[side].lights[2][i] = millis();
    }
    for (byte i = 0; i < 6; i++)
    {
      int tmp;
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
          colors11[enviroments[side].lights[3][i]] = red;
        }
        else
        {
          colors12[enviroments[side].lights[3][i]] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[enviroments[side].lights[3][i]] = blue;
        }
        else
        {
          colors12[enviroments[side].lights[3][i]] = blue;
        }
      }
    }
    if (side == FRONT)
    {
      ledStrip11.write(colors11, LED_COUNT);
    }
    else
    {
      ledStrip12.write(colors12, LED_COUNT);
    }
  }
  for (byte i = 0; i < 6; i++)
  {
    if (millis() > (long)(enviroments[side].lights[2][i] + enviroments[side].lights[1][i]))
    {
      if (side == FRONT)
      {
        colors11[enviroments[side].lights[3][i]] = off;
      }
      else
      {
        colors12[enviroments[side].lights[3][i]] = off;
      }
      int tmp;
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
          colors11[enviroments[side].lights[3][i]] = red;
        }
        else
        {
          colors12[enviroments[side].lights[3][i]] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[enviroments[side].lights[3][i]] = blue;
        }
        else
        {
          colors12[enviroments[side].lights[3][i]] = blue;
        }
      }
      enviroments[side].lights[1][i] = (int)random(enviroments[side].minDelay, enviroments[side].maxDelay + 1);
      enviroments[side].lights[2][i] = millis();
      if (side == FRONT)
      {
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        ledStrip12.write(colors12, LED_COUNT);
      }
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
          colors11[i] = red;
        }
        else
        {
          colors12[i] = red;
        }
      }
    }
    else
    {
      for (byte i = 0; i < LED_COUNT; i++)
      {
        if (side == FRONT)
        {
          colors11[i] = white;
        }
        else
        {
          colors12[i] = white;
        }
      }
    }
    //Write the appropriate array to the appropriate LED strip.
    if (side == FRONT)
    {
      ledStrip11.write(colors11, LED_COUNT);
    }
    else
    {
      ledStrip12.write(colors12, LED_COUNT);
    }
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
          colors11[i] = red;
        }
        else
        {
          colors12[i] = red;
        }
      }
      for (byte i = 1; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          colors11[i] = white;
        }
        else
        {
          colors12[i] = white;
        }
      }
    }
    else
    {
      for (byte i = 0; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          colors11[i] = white;
        }
        else
        {
          colors12[i] = white;
        }
      }
      for (byte i = 1; i < LED_COUNT; i += 2)
      {
        if (side == FRONT)
        {
          colors11[i] = red;
        }
        else
        {
          colors12[i] = red;
        }
      }
    }
    if (side == FRONT)
    {
      ledStrip11.write(colors11, LED_COUNT);
    }
    else
    {
      ledStrip12.write(colors12, LED_COUNT);
    }

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
          colors11[0] = white;
          colors11[4] = white;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[0] = white;
          colors12[4] = white;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[0] = off;
          colors11[4] = off;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[0] = off;
          colors12[4] = off;
          ledStrip12.write(colors12, LED_COUNT);
        }
        colors11[0] = off;
        colors11[4] = off;
        ledStrip11.write(colors11, LED_COUNT);
      }
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          colors11[5] = red;
          colors11[9] = red;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[5] = red;
          colors12[9] = red;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[5] = off;
          colors11[9] = off;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[5] = off;
          colors12[9] = off;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
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
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      for (byte i = 6; i < 9; i++)
      {
        if (side == FRONT)
        {
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      if (side == FRONT)
      {
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        ledStrip12.write(colors12, LED_COUNT);
      }
      enviroments[side].counters[1] = 1;
    }
    if (enviroments[side].counters[1] < 4)
    {
      if (side == FRONT)
      {
        colors11[enviroments[side].counters[1]] = yellow;
      }
      else
      {
        colors12[enviroments[side].counters[1]] = yellow;
      }
    }
    else
    {
      if (side == FRONT)
      {
        colors11[enviroments[side].counters[1] + 2] = yellow;
      }
      else
      {
        colors12[enviroments[side].counters[1] + 2] = yellow;
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
          colors11[0] = white;
          colors11[4] = white;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[0] = white;
          colors12[4] = white;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[0] = off;
          colors11[4] = off;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[0] = off;
          colors12[4] = off;
          ledStrip12.write(colors12, LED_COUNT);
        }
        colors11[0] = off;
        colors11[4] = off;
        ledStrip11.write(colors11, LED_COUNT);
      }
    }
    else if (enviroments[side].counters[0] > 6)
    {
      if ((enviroments[side].counters[0] % 2) == 1)
      {
        if (side == FRONT)
        {
          colors11[5] = red;
          colors11[9] = red;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[5] = red;
          colors12[9] = red;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[5] = off;
          colors11[9] = off;
          ledStrip11.write(colors11, LED_COUNT);
        }
        else
        {
          colors12[5] = off;
          colors12[9] = off;
          ledStrip12.write(colors12, LED_COUNT);
        }
      }
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
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      for (byte i = 6; i < 8; i++)
      {
        if (side == FRONT)
        {
          colors11[i] = off;
        }
        else
        {
          colors12[i] = off;
        }
      }
      if (side == FRONT)
      {
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        ledStrip12.write(colors12, LED_COUNT);
      }
      enviroments[side].counters[1] = 1;
    }
    if (enviroments[side].counters[1] < 4)
    {
      if (side == FRONT)
      {
        colors11[9 - enviroments[side].counters[1]] = yellow;
      }
      else
      {
        colors12[9 - enviroments[side].counters[1]] = yellow;
      }
    }
    else
    {
      if (side == FRONT)
      {
        colors11[7 - enviroments[side].counters[1]] = yellow;
      }
      else
      {
        colors12[7 - enviroments[side].counters[1]] = yellow;
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
      int tmp;
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
          colors11[enviroments[side].lights[3][i]] = red;
        }
        else
        {
          colors12[enviroments[side].lights[3][i]] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[enviroments[side].lights[3][i]] = white;
        }
        else
        {
          colors12[enviroments[side].lights[3][i]] = white;
        }
      }
    }
    if (side == FRONT)
    {
      ledStrip11.write(colors11, LED_COUNT);
    }
    else
    {
      ledStrip12.write(colors12, LED_COUNT);
    }
  }
  for (byte i = 0; i < 6; i++)
  {
    if (millis() > (long)(enviroments[side].lights[2][i] + enviroments[side].lights[1][i]))
    {
      if (side == FRONT)
      {
        colors11[enviroments[side].lights[3][i]] = off;
      }
      else
      {
        colors12[enviroments[side].lights[3][i]] = off;
      }
      int tmp;
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
          colors11[enviroments[side].lights[3][i]] = red;
        }
        else
        {
          colors12[enviroments[side].lights[3][i]] = red;
        }
      }
      else
      {
        if (side == FRONT)
        {
          colors11[enviroments[side].lights[3][i]] = white;
        }
        else
        {
          colors12[enviroments[side].lights[3][i]] = white;
        }
      }
      enviroments[side].lights[1][i] = (int)random(enviroments[side].minDelay, enviroments[side].maxDelay + 1);
      enviroments[side].lights[2][i] = millis();
      if (side == FRONT)
      {
        ledStrip11.write(colors11, LED_COUNT);
      }
      else
      {
        ledStrip12.write(colors12, LED_COUNT);
      }
    }
  }
}
