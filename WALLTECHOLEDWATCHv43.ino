/****************************************************************************************
 * ___       __   ________  ___       ___   _________  _______   ________  ___  ___     
 * |\  \     |\  \|\   __  \|\  \     |\  \ |\___   ___\\  ___ \ |\   ____\|\  \|\  \    
 * \ \  \    \ \  \ \  \|\  \ \  \    \ \  \\|___ \  \_\ \   __/|\ \  \___|\ \  \\\  \   
 *  \ \  \  __\ \  \ \   __  \ \  \    \ \  \    \ \  \ \ \  \_|/_\ \  \    \ \   __  \  
 *   \ \  \|\__\_\  \ \  \ \  \ \  \____\ \  \____\ \  \ \ \  \_|\ \ \  \____\ \  \ \  \ 
 *    \ \____________\ \__\ \__\ \_______\ \_______\ \__\ \ \_______\ \_______\ \__\ \__\
 *     \|____________|\|__|\|__|\|_______|\|_______|\|__|  \|_______|\|_______|\|__|\|__|
 * 
 * This is the default code for the Walltech OLED Watch v4.3
 * 
 * Designed specifically to work with the Walltech OLED Watch v4.3:
 * ----> http://wp.me/P4qDhv-fm
 * 
 * Walltech invests time and resources providing this open source code, 
 * please support Walltech and open-source hardware by donating any amount!
 * https://www.paypal.com/us/cgi-bin/webscr?cmd=_flow&SESSION=LrIWifBuf102Rn7Ang7qJLttsEWICxRtd9iYW6zG0OBBt_zN2z6srd7XQXa&dispatch=5885d80a13c0db1f8e263663d3faee8d6cdb53fcfca2b5941339e576d7e42259
 * 
 * Written by John Wall for Walltech.  
 * BSD license, all text above must be included in any redistribution
 * 
 * Creative Commons License
 * This work by Walltech Industries is licensed
 * under a Creative Commons Attribution-ShareAlike 4.0 International License.
 * Largely based on a work at http://www.arduino.cc
 ******************************************************************************/

#include <Wire.h>// Including the necessary libraries
#include <GOFi2cOLED.h>// This library can be found here - http://www.geekonfire.com/wiki/index.php?title=I2C_OLED_Panel(128x64)
#include "RTClib.h"//These are Adafruit libraries, support them at www.adafruit.com!
#include "Adafruit_MCP9808.h"

#define MONITOR_BUTTONS 8 //passing this to buttonState will return button states
#define RIGHT_BUTTON 0  //each number represents the bit that represents the button
#define CENTER_BUTTON 1 //that is a bit in the byte buttonState returns
#define LEFT_BUTTON 2
// !! --> all delays need to be removed for button update to work properly <--- !!
//buttonUpdate(); //update the debounced reading
// is right pressed?
//if(bitRead(buttonState(MONITOR_BUTTONS),RIGHT_BUTTON)) 
//{doRightButtonStuff();}


// seperate tabs for the display, clock and tempsensor functions would be nice
GOFi2cOLED GOFoled;// instantiating constructors 
RTC_DS1307 RTC;
//Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();


/* !! remove as many of these global vars as you can !!***
      they are only going to create head aches for you 
      as time goes on
      
      on lookers like myself have no idea what is going
      and will doubt you have any idea either.
      
      You can restrict a persistant var to a function by declaring it as
      "static" inside of the function
      
      see buttonUpdate and buttonState for examples of this
      
      Think the temp sensor function could have 
      static float temperature = 0; // only declares 0 first function call
      when inside the fuction temperature is set to 98 
      this persist till function changes it
      regardless of exiting the function
*/


byte timehour;// declaring all program variables
byte timeminute;
byte timesecond;
byte lastsecond;

int timemillisecond;
byte PROGMEM charge = 3;
byte PROGMEM full = A3;
byte brightness = 0;// how bright the charging LED is
byte fadeAmount = 5;// how many points to fade the charging LED by
byte ledstate = LOW;
int lipostat;//Charge status
byte percent;

float f;

int value = LOW;// previous value of the LED
int PROGMEM stopwatchblink = 1000;
int stopButtonState;
int laststopButtonState;// variable to store last button state
byte blinking;// condition for blinking - timer is timing
long previousMillis = 0;// variable to store last time LED was updated
long startTime;// start time for stop watch
long elapsedTime;// elapsed time for stop watch
long runningTime;
int fractional;// variable used to store fractional part of time
byte hours;
byte minutes;
byte seconds;
long previousmillisseconds;

byte setminute;
byte sethour;

long PROGMEM interval = 333;

byte face = 1;
byte PROGMEM facenum = 9;

byte brightnessLevel = 4;
long fadeMillis;

byte A = 1;
byte B = 1;
byte C = 1;
byte D = 1;

// !! pick a coding style !!
// mine is unconventional/unpopular maybe, but it is consistent
// where ever you decide to put your brackets always use em and be consistent about it
// Noticed you had/have alot of white space - giving the open bracket its own line might
// be for you then  

void setup()
{
  GOFoled.init(0x3c);// initiate the screen at its address
  GOFoled.clearDisplay();

  Wire.begin();// Beginning the library functions

  RTC.begin();

  tempsensor.begin();

  //////////////
  //RTC.adjust(DateTime(__DATE__, __TIME__));// Setting the DS1307 time
  //////////////
  //**NOTE** 
  // After uploading, comment out the above line
  // and re-upload. This is very 
  // important because without it,
  // the time will reset to the original set
  // time every power cycle instead 
  // of keeping with the current time.
  
  buttonUp(); //initialize the buttons

  pinMode(A7, INPUT);// sense pins and output LEDs set
  pinMode(A2, INPUT_PULLUP);
  pinMode(charge, OUTPUT);
  pinMode(full, OUTPUT);
  
  GOFoled.setRotation(2);// the screen is actually mounted upside down, so we flip it
  GOFoled.setBrightness(255);// full brightness to start
  GOFoled.drawBitmap(1, 25, walltech2 ,126, 15, WHITE);// logo and startup animation
  GOFoled.display();
  delay(1000);             //<-- !! delays in the set up are fine
  GOFoled.setCursor(52,57);
  GOFoled.print(F("v4.3"));
  GOFoled.display();
  delay(1000);
  GOFoled.clearDisplay();

  GOFoled.setCursor(38,7);
  GOFoled.print(F("John Wall"));
  GOFoled.setCursor(25,29);
  GOFoled.print(F("@walltechOSHW"));
  GOFoled.setCursor(20,50);
  GOFoled.print(F("www.walltech.cc"));
  GOFoled.display();
  delay(1000);
  GOFoled.clearDisplay();

  fadeMillis = millis();
  checkBatteryLevel();
}

void loop() //would benefit some of this being broken out into functions !!
{
  DateTime now = RTC.now();// the time variables are redefined

  timehour   = now.hour();
  timeminute = now.minute();
  timesecond = now.second();
  if (timehour > 12) {timehour = timehour - 12;}
  // comment these two ifs to convert to 24 hour time (haven't tested that)
  if (now.hour() == 0){timehour = 12;}

  buttonUpdate();// the states of the buttons are read
  
  lipostat = analogRead(A2);// the charge status of the battery is measured

  if(buttonState_r == LOW)
  {// if the right button is pressed, advance the faces by one and reset the sleep counter
    delay(100); //<-- !! a delay here is going to cause problems !!
    face++; 
    fadeMillis = millis();
    checkBatteryLevel();
  }
  if(buttonState_c == LOW)
  {// if the center button is pressed just reset the sleep counter
    delay(100);
    fadeMillis = millis();
    checkBatteryLevel();
  }
  if(buttonState_l == LOW)
  {// left scroll
    delay(100);
    face--; 
    fadeMillis = millis();
    checkBatteryLevel();
  }
  // loop the faces back up again
  if(face < 1){face = facenum;}
  if(face > facenum){face = 1;}
  if(face == 1)
  {// first face
    if(timehour<10)
    {
      displayNum(18,18,timehour);// print the hour's number

      if(now.second() % 2 == 0)// flash the colon once a second
      {
        GOFoled.drawBitmap(49, 23, fontcolon, 3, 21, WHITE);
      }
      else
      {
        GOFoled.drawRect(49, 23, 3, 21, BLACK);
      }

      if(timeminute<10)// add a leading zero if the minute's less than 10
      {
        displayNum(56,18,0);
        displayNum(87,18,timeminute);
      }
      else
      {
        displayNum(56,18,timeminute/10);
        displayNum(87,18,timeminute % 10);
      }
    }
    else
    {
      displayNum(0,18,timehour/10);
      displayNum(31,18,timehour % 10);

      if(now.second() % 2 == 0)
      {
        GOFoled.drawBitmap(61, 23, fontcolon, 3, 21, WHITE);
      }
      else{GOFoled.drawRect(61, 23, 3, 21, BLACK);}

      if(timeminute<10)
      {
        displayNum(69,18,0);
        displayNum(100,18,timeminute);
      }
      else
      {
        displayNum(69,18,timeminute/10);
        displayNum(100,18,timeminute % 10);
      }
    }
    GOFoled.setCursor(10,57);// set cursor and print the date
    GOFoled.setTextSize(1);
    if (now.dayOfWeek() == 0)     {GOFoled.print(F("Sun  "));}
    else if (now.dayOfWeek() == 1){GOFoled.print(F("Mon  "));}
    else if (now.dayOfWeek() == 2){GOFoled.print(F("Tue  "));}
    else if (now.dayOfWeek() == 3){GOFoled.print(F("Wed  "));}
    else if (now.dayOfWeek() == 4){GOFoled.print(F("Thu  "));}
    else if (now.dayOfWeek() == 5){GOFoled.print(F("Fri  "));}
    else if (now.dayOfWeek() == 6){GOFoled.print(F("Sat  "));}

    GOFoled.print(now.month());
    GOFoled.print(F("/"));
    GOFoled.print(now.day());
    GOFoled.print(F("/"));    
    GOFoled.print(now.year());  

    GOFoled.print(F(" "));
    GOFoled.print(percent);
    GOFoled.print(F("%"));
  }
  else if(face == 2)// printing each number of the time as the corresponding bitmap of the font
  {
    if(timehour < 10)
    {
      blindNum(25,0,timehour%10);
      blindNum(57,0,timeminute/10);
      blindNum(81,0,timeminute%10); 

      if(timesecond % 2 == 0)
      {GOFoled.drawBitmap(48, 0, blindcolon ,8, 64, WHITE);}
    }
    else
    {
      blindNum(1,0,timehour/10);
      blindNum(25,0,timehour%10);
      blindNum(72,0,timeminute/10);
      blindNum(102,0,timeminute%10);
      
      if(timesecond % 2 == 0) 
      {GOFoled.drawBitmap(56, 0, blindcolon ,8, 64, WHITE);}
    }
  }

  else if (face == 3)// optical illusion box font
  {
    if(timehour < 10)
    {
      displayBoxFont(15,0,timehour%10);
      displayBoxFont(53,0,timeminute/10);
      displayBoxFont(83,0,timeminute%10); 

      if(timesecond % 2 == 0){GOFoled.drawBitmap(45, 0, boxcolon ,8, 64, WHITE);}
      else {GOFoled.drawBitmap(45, 0, boxcoloninverted ,8, 64, WHITE);}
    }
    else
    {
      displayBoxFont(0,0,timehour/10);
      displayBoxFont(30,0,timehour%10);
      displayBoxFont(68,0,timeminute/10);
      displayBoxFont(98,0,timeminute%10); 
      if(timesecond % 2 == 0) {GOFoled.drawBitmap(60, 0, boxcolon ,8, 64, WHITE);}
      else{GOFoled.drawBitmap(60, 0, boxcoloninverted ,8, 64, WHITE);}
    } 
  }
  else if(face == 4)
  {// the bitmaps are text, same as before pretty much
    GOFoled.drawBitmap(0, 0, its, 72, 16, WHITE);
    if(timeminute == 0)
    {
      textNum(0,24,timehour);
      GOFoled.drawBitmap(0, 48, oclock, 128, 16, WHITE);
    }
    else if(timeminute > 0 && timeminute < 10)
    {
      textNum(0,24,timehour);
      GOFoled.drawBitmap(0, 48, oh, 72, 16, WHITE);
      textNum(24,48, timeminute);
    }
    else if(timeminute > 9 && timeminute < 20)
    {
      textNum(0,24,timehour);
      textNum(0,48,timeminute);
    }
    else if(timeminute >= 20 && timeminute% 10 == 0)
    {
      textNum(0,24,timehour);

      if(timeminute/10 == 2) GOFoled.drawBitmap(0, 48, twenty, 72, 16, WHITE);
      else if(timeminute/10 == 3) GOFoled.drawBitmap(0, 48, thirty, 72, 16, WHITE);
      else if(timeminute/10 == 4) GOFoled.drawBitmap(0, 48, forty, 72, 16, WHITE);
      else if(timeminute/10 == 5) GOFoled.drawBitmap(0, 48, fifty, 72, 16, WHITE);
    }
    else if(timeminute > 20 && timeminute% 10 != 0)
    {
      textNum(0,16,timehour);
      if(timeminute/10 == 2) GOFoled.drawBitmap(0, 32, twenty, 72, 16, WHITE);
      else if(timeminute/10 == 3) GOFoled.drawBitmap(0, 32, thirty, 72, 16, WHITE);
      else if(timeminute/10 == 4) GOFoled.drawBitmap(0, 32, forty, 72, 16, WHITE);
      else if(timeminute/10 == 5) GOFoled.drawBitmap(0, 32, fifty, 72, 16, WHITE);
      textNum(0,48, timeminute%10);
    }
  }
  else if(face == 5)
  {// this stuff does the math necessary for the analog clock face
    GOFoled.drawRect( 0, 0,  128, 64, WHITE );
    GOFoled.drawFastVLine( 62, 2, 4, WHITE );
    GOFoled.drawFastVLine( 64, 2, 4, WHITE );
    GOFoled.drawFastHLine( 120, 32, 6, WHITE );
    GOFoled.drawFastVLine( 63, 58, 4, WHITE );
    GOFoled.drawFastHLine( 2, 32, 6, WHITE );

    GOFoled.drawPixel( 79, 4, WHITE );
    GOFoled.drawPixel( 113,4, WHITE);
    GOFoled.drawPixel( 113, 59, WHITE );
    GOFoled.drawPixel( 79, 59, WHITE );
    GOFoled.drawPixel( 47, 59, WHITE );
    GOFoled.drawPixel( 15, 59, WHITE );
    GOFoled.drawPixel( 15, 4, WHITE );
    GOFoled.drawPixel( 47, 4, WHITE );

    GOFoled.drawLine( 63, 32, 63 + 25 * cos( ( 270 + now.second() * 6 ) * 3.14159 / 180 ), 32 + 25 * sin( ( 270 + now.second() * 6 ) * 3.14159 / 180 ), WHITE );    

    GOFoled.drawLine( 63, 32, 63 + 20 * cos( ( 270 + now.minute() * 6 ) * 3.14159 / 180 ), 32 + 20 * sin( ( 270 + now.minute() * 6 ) * 3.14159 / 180 ), WHITE );    

    GOFoled.drawLine( 63, 32, 63 + 15 * cos( ( 270 + now.hour() % 12 * 30 + now.minute() * 0.5 ) * 3.14159 / 180 ),32 + 15 * sin( ( 270 + now.hour() % 12 * 30 + now.minute() * 0.5 ) * 3.14159 / 180 ), WHITE );
    //for readabilty sake braking these parameter entries into functions would help !!
  }
  else if(face == 6)//temperature sensor
  {
    if(A == 1) 
    {
      GOFoled.drawBitmap(46, 14, temp, 36, 36,WHITE);
      GOFoled.display();
      delay(1000);
      GOFoled.clearDisplay();
      A++;
    }

    GOFoled.setCursor(28,7);
    GOFoled.print(F("Temperature:"));
    GOFoled.setCursor(10,29);
    GOFoled.print(getTemp());
    GOFoled.print(F("F"));
    GOFoled.display();
  }
  else if(face == 7)
  {// stopwatch logic
    if(B == 1)
    {
      GOFoled.drawBitmap(46, 12, stopwatch, 36, 40,WHITE);
      GOFoled.display();
      delay(1000);
      GOFoled.clearDisplay();
      B++;
    }

    GOFoled.setTextSize(1);
    GOFoled.setCursor(38,57);
    GOFoled.print(F("Stopwatch"));

    GOFoled.setTextSize(2);
    GOFoled.setTextColor(WHITE);
    GOFoled.setCursor(2,25);

    // check for button press
    stopButtonState = digitalRead(cbtn);                   // read the button state and store

    if (stopButtonState == LOW && laststopButtonState == HIGH  &&  blinking == false)
    {     // check for a high to low transition
      // if true then found a new button press while clock is not running - start the clock
      startTime = millis();                                   // store the start time
      blinking = true;                                     // turn on blinking while timing
      delay(5);                                               // short delay to debounce switch
      laststopButtonState = stopButtonState;                          // store stopButtonState in laststopButtonState, to compare next time
    }

    else if (stopButtonState == LOW && laststopButtonState == HIGH && blinking == true){     // check for a high to low transition
      // if true then found a new button press while clock is running - stop the clock and report
      elapsedTime =   millis() - startTime;              // store elapsed time
      blinking = false;                                  // turn off blinking, all done timing
      laststopButtonState = stopButtonState;                     // store stopButtonState in laststopButtonState, to compare next time
      displayStopwatch(elapsedTime);
    }
    else
    {
      laststopButtonState = stopButtonState;                         // store stopButtonState in laststopButtonState, to compare next time
      if(blinking == true)
      {
        runningTime = millis() - startTime; 
        displayStopwatch(runningTime);
      }
      if(blinking == false){displayStopwatch(elapsedTime);}
    }
    fadeMillis = millis();
    digitalWrite(13,LOW);
  }

  else if (face == 8)
  {// timeset
    if(C == 1)
    {
      setminute = now.minute();
      sethour = now.hour();
      GOFoled.drawBitmap(46, 14, settime, 36, 36,WHITE);
      GOFoled.display();
      delay(1000);
      GOFoled.clearDisplay();
      C++;
    }

    checkButtons();

    GOFoled.setCursor(16,49);
    GOFoled.setTextSize(1);
    GOFoled.print(F("Hold to set time"));
    GOFoled.setCursor(57,57);
    GOFoled.print(F("|"));

    while(buttonState_c == LOW)
    {// while the center button is held
      GOFoled.clearDisplay();
      checkButtons();
      GOFoled.setTextSize(2);
      GOFoled.setTextColor(WHITE);
      GOFoled.setCursor(20,0);
      GOFoled.println(F("Set Time"));
      GOFoled.setCursor(0,40);
      GOFoled.print(F("Min-> "));
      GOFoled.print(setminute);
      GOFoled.setCursor(0,20);
      GOFoled.print(F("Hr-> "));
      GOFoled.print(sethour);
      GOFoled.setCursor(0,57);
      GOFoled.setTextSize(1);
      GOFoled.print(F("Hour           Minute"));

      if(buttonState_r == LOW)
      {// right advances the minutes       
        delay(100);
        setminute++; 
        if(setminute > 59){setminute = 0;} 
      }
      if(buttonState_l == LOW){// left the hours
        delay(100);
        sethour++;
        if(sethour > 24){sethour = 1;}
      }
      if(buttonState_c == HIGH)
      {// when the center button is released, save the newly entered time
        delay(100);
        DateTime now = RTC.now();
        DateTime updated = DateTime(now.year(), now.month(), now.day(), sethour, setminute, 0);
        RTC.adjust(updated);
        RTC.begin();
        face = 1;
        break;
      }
      GOFoled.display();
      GOFoled.clearDisplay();
    }
  }
  else if(face == 9)
  {
    if(D == 1)
    {
      setminute = now.minute();
      sethour = now.hour();
      GOFoled.drawBitmap(46, 14, brightnesssetting, 36, 36,WHITE);
      GOFoled.display();
      delay(1000);
      GOFoled.clearDisplay();
      D++;
    }
    checkButtons();

    if(buttonState_c == LOW)
    {
      delay(100);
      brightnessLevel++; 
      if(brightnessLevel > 4){brightnessLevel = 1;} 
    }

    if(brightnessLevel == 1)
    {
      GOFoled.setCursor(27,56);
      GOFoled.setTextSize(1);
      GOFoled.print(F("Brightness: 1"));
      GOFoled.drawBitmap(46, 14, brightness0 ,36 , 36 ,WHITE);
      GOFoled.setBrightness(1);
    }
    else if (brightnessLevel == 2)
    {
      GOFoled.setCursor(27,56);
      GOFoled.setTextSize(1);
      GOFoled.print(F("Brightness: 2"));
      GOFoled.drawBitmap(46, 14, brightness1 ,36 , 36 ,WHITE);
      GOFoled.setBrightness(50); 
    }
    else if (brightnessLevel == 3)
    {
      GOFoled.setCursor(27,56);
      GOFoled.setTextSize(1);
      GOFoled.print(F("Brightness: 3"));
      GOFoled.drawBitmap(46, 14, brightness2 ,36 , 36 ,WHITE);
      GOFoled.setBrightness(150); 
    }
    else if (brightnessLevel == 4)
    {
      GOFoled.setCursor(27,56);
      GOFoled.setTextSize(1);
      GOFoled.print(F("Brightness: 4"));
      GOFoled.drawBitmap(46, 14, brightness3 ,36 , 36 ,WHITE);
      GOFoled.setBrightness(255); 
    }

  }
// these reset variables used to display the icons when the face changes
  if(face != 6){A = 1;}
  if(face != 7)
  {
    B = 1;
    if ( (millis() - previousMillis > 500) )
    {
      if(blinking)
      {
        previousMillis = millis();                         // remember the last time we blinked the LED

        // if the LED is off turn it on and vice-versa.
        if (value == LOW){value = HIGH;}
        else{value = LOW;}
        digitalWrite(13, value);
      }
    }
  }

  if (face != 8){C = 1;}
  if (face != 9){D = 1;}
  else digitalWrite(13, LOW);

  if(millis() - fadeMillis >= 10000){GOFoled.clearDisplay(); }

  ////////////////////////////////////////////////////////
  if(readVcc() > 4200)
  {
    if (lipostat < 512){// if the battery status is low, it's charging
      GOFoled.clearDisplay();
      GOFoled.setBrightness(255);
      GOFoled.drawBitmap(48, 16, battplug, 32, 32, WHITE);
      GOFoled.setCursor(48,57);
      GOFoled.print(readVcc());
      GOFoled.print(F("mV"));
      analogWrite(charge, brightness);    
      // change the brightness for next time through the loop:
      brightness = brightness + fadeAmount;
      // reverse the direction of the fading at the ends of the fade: 
      if (brightness == 0 || brightness == 100) {fadeAmount = -fadeAmount;}     
      {digitalWrite(full, LOW);}
    }
    else if(lipostat > 512)// if the battery state is high, and it's been plugged in and charging before this, show full
    {
      GOFoled.clearDisplay();
      GOFoled.setBrightness(255);
      GOFoled.drawBitmap(48, 16, fullbatt , 32, 32, WHITE);
      digitalWrite(full, HIGH);
      digitalWrite(charge, LOW);
    }
  }
  else
  {
    digitalWrite(charge, LOW); 
    digitalWrite(full, LOW); 
  }

  while (readVcc() <= 3200){// if the power gets so low that the DS1307 glitches, the battery is dead, battery dead animation shown
    GOFoled.setBrightness(255);
    GOFoled.clearDisplay();
    GOFoled.drawBitmap(48, 16, emptybatt, 32, 32, WHITE);
    delay(250);
  }
  GOFoled.display();
  GOFoled.clearDisplay();
} // <--- I think this this the end of the main loop?

// font displaying methods and other program methods

void textNum(int x, int y, int n) 
{// !! make an array of pointers to number images
 // this way they can be accessed as 0,1,2,3 and so on 
 // GOFoled.drawBitmap(x, y, PGM_READ_WHATEVER(&NUMBER_ARRAY[n], 72, 16, WHITE) <-- !!! one line !!!
 // im sure the folks on the forums could help you creat a array like that, not my forte
  if(n == 1)       {GOFoled.drawBitmap(x, y, one, 72, 16, WHITE);}
  else if ( n == 2){GOFoled.drawBitmap(x, y, two, 72, 16, WHITE);}
  else if ( n == 3){GOFoled.drawBitmap(x, y, three, 72, 16, WHITE);}
  else if ( n == 4){GOFoled.drawBitmap(x, y, four, 72, 16, WHITE);}
  else if ( n == 5){GOFoled.drawBitmap(x, y, five, 72, 16, WHITE);}
  else if ( n == 6){GOFoled.drawBitmap(x, y, six, 72, 16, WHITE);}
  else if ( n == 7){GOFoled.drawBitmap(x, y, seven, 72, 16, WHITE);}
  else if ( n == 8){GOFoled.drawBitmap(x, y, eight, 72, 16, WHITE);}
  else if ( n == 9){GOFoled.drawBitmap(x, y, nine, 72, 16, WHITE);}
  else if ( n == 10){GOFoled.drawBitmap(x, y, ten, 72, 16, WHITE);}
  else if ( n == 11){GOFoled.drawBitmap(x, y, eleven, 72, 16, WHITE);}
  else if ( n == 12){GOFoled.drawBitmap(x, y, twelve, 72, 16, WHITE);}
  else if ( n == 13){GOFoled.drawBitmap(x, y, thirteen, 128, 16, WHITE);}
  else if ( n == 14){GOFoled.drawBitmap(x, y, fourteen, 128, 16, WHITE);}
  else if ( n == 15){GOFoled.drawBitmap(x, y, fifteen, 128, 16, WHITE);}
  else if ( n == 16){GOFoled.drawBitmap(x, y, sixteen, 128, 16, WHITE);}
  else if ( n == 17){GOFoled.drawBitmap(x, y, seventeen, 128, 16, WHITE);}
  else if ( n == 18){GOFoled.drawBitmap(x, y, eighteen, 128, 16, WHITE);}
  else if ( n == 19){GOFoled.drawBitmap(x, y, nineteen, 128, 16, WHITE);}
}

void displayNum(int x, int y, int n)
{
  if(n == 0)     {GOFoled.drawBitmap(x, y, font0, 25, 28, WHITE);}
  else if(n == 1){GOFoled.drawBitmap(x, y, font1, 25, 28, WHITE);}
  else if(n == 2){GOFoled.drawBitmap(x, y, font2, 25, 28, WHITE);}
  else if(n == 3){GOFoled.drawBitmap(x, y, font3, 25, 28, WHITE);}
  else if(n == 4){GOFoled.drawBitmap(x, y, font4, 25, 28, WHITE);}
  else if(n == 5){GOFoled.drawBitmap(x, y, font5, 25, 28, WHITE);}
  else if(n == 6){GOFoled.drawBitmap(x, y, font6, 25, 28, WHITE);}
  else if(n == 7){GOFoled.drawBitmap(x, y, font7, 25, 28, WHITE);}
  else if(n == 8){GOFoled.drawBitmap(x, y, font8, 25, 28, WHITE);}
  else if(n == 9){GOFoled.drawBitmap(x, y, font9, 25, 28, WHITE);}
}

void blindNum(int x, int y, int n)
{
  if(n == 0)     {GOFoled.drawBitmap(x, y, blind0, 22, 64, WHITE);}
  else if(n == 1){GOFoled.drawBitmap(x, y, blind1, 22, 64, WHITE);}
  else if(n == 2){GOFoled.drawBitmap(x, y, blind2, 22, 64, WHITE);}
  else if(n == 3){GOFoled.drawBitmap(x, y, blind3, 22, 64, WHITE);}
  else if(n == 4){GOFoled.drawBitmap(x, y, blind4, 22, 64, WHITE);}
  else if(n == 5){GOFoled.drawBitmap(x, y, blind5, 22, 64, WHITE);}
  else if(n == 6){GOFoled.drawBitmap(x, y, blind6, 22, 64, WHITE);}
  else if(n == 7){GOFoled.drawBitmap(x, y, blind7, 22, 64, WHITE);}
  else if(n == 8){GOFoled.drawBitmap(x, y, blind8, 22, 64, WHITE);}
  else if(n == 9){GOFoled.drawBitmap(x, y, blind9, 22, 64, WHITE);}
}

void displayBoxFont(int x, int y, int n)
{
  if(n == 0)     {GOFoled.drawBitmap(x, y, box0, 30, 64, WHITE);}
  else if(n == 1){GOFoled.drawBitmap(x, y, box1, 30, 64, WHITE);}
  else if(n == 2){GOFoled.drawBitmap(x, y, box2, 30, 64, WHITE);}
  else if(n == 3){GOFoled.drawBitmap(x, y, box3, 30, 64, WHITE);}
  else if(n == 4){GOFoled.drawBitmap(x, y, box4, 30, 64, WHITE);}
  else if(n == 5){GOFoled.drawBitmap(x, y, box5, 30, 64, WHITE);}
  else if(n == 6){GOFoled.drawBitmap(x, y, box6, 30, 64, WHITE);}
  else if(n == 7){GOFoled.drawBitmap(x, y, box7, 30, 64, WHITE);}
  else if(n == 8){GOFoled.drawBitmap(x, y, box8, 30, 64, WHITE);}
  else if(n == 9){GOFoled.drawBitmap(x, y, box9, 30, 64, WHITE);}
}

void displayStopwatch(long input)
{
  hours = (int)(input / 3600000L);
  minutes = (int)(input / 60000L) % 60;
  seconds = (int)(input / 1000L) % 60;

  if(hours < 10){GOFoled.print(F("0"));}

  GOFoled.print(hours); 

  GOFoled.print(F(":"));                             // print colon

  if( minutes < 10){GOFoled.print(F("0"));}

  GOFoled.print(minutes); 

  GOFoled.print(F(":"));                             // print colon

  if(seconds < 10){GOFoled.print(F("0"));}

  GOFoled.print(seconds);         

  GOFoled.print(F(":"));                             // print colon

  // use modulo operator to get fractional part of time 
  fractional = (int)(input % 1000L);
  GOFoled.setTextSize(1);
  // pad in leading zeros - wouldn't it be nice if 
  // Arduino language had a flag for this? :)
  if (fractional == 0)
    {GOFoled.print(F("00"));}      // add three zero's
  else if (fractional < 10)    // if fractional < 10 the 0 is ignored giving a wrong time, so add the zeros
    {GOFoled.print(F("00"));}      // add two zeros
  else if (fractional < 100)
    {GOFoled.print(F("0"));}        // add one zero
  GOFoled.print(fractional);  // print fractional part of time 
}

void checkButtons()
{
  buttonState_l = digitalRead(lbtn);
  buttonState_c = digitalRead(cbtn);
  buttonState_r = digitalRead(rbtn);
}

int readVcc()
{
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return (int)result;
}

void checkBatteryLevel()
{
  percent = map(readVcc(), 3780, 3125, 100, 0);
  if(percent > 100){percent = 100;}
}

double getTemp()
{
  f = (tempsensor.readTempC() * 9.0 / 5.0 + 32)-(double)((int)(getTemp()/0.1)%10);
  return f;
}
